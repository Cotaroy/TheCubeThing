#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include "camera.h"
#include "manager.h"
#include "raycast.h"
#include "renderer.h"
#include "space.h"
#define PI (3.14159265358979323846)



static void normalise_vector_mutate(double *x, double *y, double *z) {
    double length = sqrt((*x)*(*x) + (*y)*(*y) + (*z)*(*z));
    *x /= length;
    *y /= length;
    *z /= length;
}

/**
 * According to the man page on `read`, it sometimes doesn't read everything you
 * told it to read, even if the data is there and readable. This function is
 * supposed to prevent this funniness.
 *
 * Returns -1 if something went wrong.
 * Returns the number of bytes read if not.
 * A return value of 0 indicates EOF.
 */
static ssize_t read_safely(
        int source_file_descriptor,
        void *source_buffer,
        size_t num_bytes_wanted) {
    size_t nbytes_successful = 0;
    while(nbytes_successful < num_bytes_wanted) {
        ssize_t nbytes_now = read(
            source_file_descriptor, 
            source_buffer + nbytes_successful, 
            num_bytes_wanted - nbytes_successful);

        if(nbytes_now == 0) {
            // end of file reached
            return 0;
        }
        if(nbytes_now < 0) {
            if(errno == EINTR) {
                // interrupted by a signal
                // see `man 2 read` and `man 7 signal`
                continue; // keep trying to read more bytes
            }
            return -1; // give up
        }

        nbytes_successful += nbytes_now;
    }
    return nbytes_successful;
}

/**
 * Prevent funny business with `write` not writing the whole thing even though
 * there's space because of a signal interrupt or something.
 *
 * Returns -1 if something went wrong.
 * Returns the number of bytes written if not.
 */
ssize_t write_safely(int destination_file_descriptor,
                            void *source_buffer, size_t num_bytes_wanted) {
    size_t nbytes_successful = 0;
    while (nbytes_successful < num_bytes_wanted) {
        errno = 0;
        
        ssize_t nbytes_now = write(destination_file_descriptor,
                                   source_buffer + nbytes_successful,
                                   num_bytes_wanted - nbytes_successful);

        // if (nbytes_now == 0) {
        //     // end of file reached
        //     return 0;
        // }
        if (nbytes_now < 0) {
            if (errno == EINTR) {
                // interrupted by a signal
                // see `man 2 write` and `man 7 signal`
                continue; // keep trying to write more bytes
            }
            if(errno == EPIPE) {
                fprintf(stderr, "borken pipe\n");
                return SENTINEL_WRITE_SAFELY_BROKEN_PIPE;
            }
            return -1; // give up
        }

        nbytes_successful += nbytes_now;
    }
    return nbytes_successful;
}

static void cleanup_child(int fd_read, int fd_write, EntitySpace *space) {
    if (close(fd_read) == -1) {
        perror("close");
        free_space(space);
        exit(1);
    }
    if (close(fd_write) == -1) {
        perror("close");
        free_space(space);
        exit(1);
    }
    free_space(space);
}

static int fd_read_for_sig_handler;
static int fd_write_for_sig_handler;
static EntitySpace *space_for_sig_handler;

static void cleanup_child_sig() {
    if (close(fd_read_for_sig_handler) == -1) {
        perror("close");
        free_space(space_for_sig_handler);
        exit(1);
    }
    if (close(fd_write_for_sig_handler) == -1) {
        perror("close");
        free_space(space_for_sig_handler);
        exit(1);
    }
    free_space(space_for_sig_handler);
}

void camera_worker_work(
        int worker_idx,
        int fd_read,
        int fd_write,
        EntitySpace *space) {

    // overwritten repeatedly to store stuff that is read in from the pipe
    CameraMessageHeader header;
    CameraRaycastTask task;
    CameraRaycastTaskResult result;


    int tasks_completed = 0;
    while(1) {
        int read_result;

        // try to read the header for a message.
        // This should block until there is actually something to read.
        read_result =
            read_safely(fd_read, &header, sizeof(CameraMessageHeader));
        if (read_result == -1) {
            perror("read");
            cleanup_child(fd_read, fd_write, space);
            exit(1);
        }
        if (read_result == 0) {
            // EOF, meaning the write end has been closed
            cleanup_child(fd_read, fd_write, space);
            exit(0);
        }

        // printf("Worker [%d] received header of type 0x%x\n", worker_idx, header.message_type);

        // do different things depending on the message type
        switch(header.message_type) {

        case MSGTYPE_RAYCAST_TASK:
            read_result =
                read_safely(fd_read, &task, sizeof(CameraRaycastTask));
            if (read_result == -1) {
                perror("read");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }

            double pos[3] = {task.ray_origin_x, task.ray_origin_y,
                             task.ray_origin_z};
            // printf("parameters to shoot_ray:
            // pos=(%lf,%lf,%lf),azim=%lf,incl=%lf",
            //         pos[0], pos[1], pos[2], task.ray_azimuth,
            //         task.ray_inclination);

            normalise_vector_mutate(&task.ray_direction_x,
                                    &task.ray_direction_y,
                                    &task.ray_direction_z);
            double distance =
                shoot_light_ray(pos, task.ray_direction_x, task.ray_direction_y,
                          task.ray_direction_z, space);

            // printf("[%d] Distance: %lf\n", worker_idx, distance);
            result.image_x = task.image_x;
            result.image_y = task.image_y;
            result.distance = distance;
            if (write_safely(fd_write, &result, sizeof(result)) <= 0) {
                perror("child write");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }
            // printf("child wrote result for pixel (%d, %d)\n",
            // result->image_x, result->image_y);

            tasks_completed++;

            break;

        case MSGTYPE_SPACE_UPDATE_TRANSLATE_ENTITY:
            CameraWorkerSpaceUpdate_TranslateEntity details;
            if (read_safely(fd_read, &details,
                            sizeof(CameraWorkerSpaceUpdate_TranslateEntity)) <=
                0) {
                perror("read details of TranslateEntity update");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }
            Entity *entity = get_entity(space, details.entity_id);
            translate(entity, details.x_offset, details.y_offset,
                      details.z_offset);
            break;
        
        case MSGTYPE_SPACE_UPDATE_ROTATE_ENTITY: {
            CameraWorkerSpaceUpdate_RotateEntity details;
            if(read_safely(fd_read, &details, sizeof(CameraWorkerSpaceUpdate_RotateEntity)) <= 0) {
                perror("read details of RotateEntity update");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }
            Entity *entity = get_entity(space, details.entity_id);

            switch (details.axis_of_rotation) {
                case MSGDETAIL_ROTATE_ENTITY_AXIS_X:
                    rotate_x(entity, details.angle, details.x_center, details.y_center, details.z_center);    
                    break;
                case MSGDETAIL_ROTATE_ENTITY_AXIS_Y:
                    rotate_y(entity, details.angle, details.x_center, details.y_center, details.z_center);    
                    break;
                case MSGDETAIL_ROTATE_ENTITY_AXIS_Z:
                    rotate_z(entity, details.angle, details.x_center, details.y_center, details.z_center);    
                    break;
                default:
                    fprintf(stderr, "Unknown rotation axis 0x%x.",
                            details.axis_of_rotation);
                    cleanup_child(fd_read, fd_write, space);
                    exit(1);
            }
            break;
        }

        case MSGTYPE_SPACE_UPDATE_TRANSLATE_LIGHTSOURCE: {
            CameraWorkerSpaceUpdate_TranslateLightSource details;
            if (read_safely(fd_read, &details, sizeof(CameraWorkerSpaceUpdate_TranslateLightSource)) <= 0) {
                perror("read details of TranslateLightSource update");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }
            LightSource *source = get_light(space, details.entity_id);
            translate_light(source, details.x_offset, details.y_offset, details.z_offset);
            break;
        }

        case MSGTYPE_SPACE_UPDATE_ROTATE_LIGHTSOURCE: {
            CameraWorkerSpaceUpdate_RotateLightSource details;
            if(read_safely(fd_read, &details, sizeof(CameraWorkerSpaceUpdate_RotateLightSource)) <= 0) {
                perror("read details of RotateLightSource update");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }
            LightSource *entity = get_light(space, details.entity_id);

            switch (details.axis_of_rotation) {
                case MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_X:
                    rotate_x_light(entity, details.angle, details.x_center, details.y_center, details.z_center);    
                    break;
                case MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_Y:
                    rotate_y_light(entity, details.angle, details.x_center, details.y_center, details.z_center);    
                    break;
                case MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_Z:
                    rotate_z_light(entity, details.angle, details.x_center, details.y_center, details.z_center);    
                    break;
                default:
                    fprintf(stderr, "Unknown rotation axis 0x%x.",
                            details.axis_of_rotation);
                    cleanup_child(fd_read, fd_write, space);
                    exit(1);
            }
            break;
        }

        case MSGTYPE_SPACE_UPDATE_BRIGHTEN_LIGHTSOURCE: {
            CameraWorkerSpaceUpdate_BrightenLightSource details;
            if (read_safely(fd_read, &details, sizeof(CameraWorkerSpaceUpdate_BrightenLightSource)) <= 0) {
                perror("read details of BrightenLightSource update");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }
            LightSource *source = get_light(space, details.entity_id);
            brighten(source, details.delta_intensity);
            break;
        }

        case MSGTYPE_SPACE_UPDATE_NEW_ENTITY: {
            CameraWorkerSpaceUpdate_NewEntity details;
            if (read_safely(fd_read, &details, sizeof(CameraWorkerSpaceUpdate_NewEntity)) <= 0) {
                perror("read details of NewEntity update");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }
            Entity *entity = create_rectangle(details.corner_coord[0], details.corner_coord[1], details.corner_coord[2], 
                                              details.side_lengths[0], details.side_lengths[1], details.side_lengths[2]);
            add_to_entity_space(space, entity, details.entity_id);
            break;
        }

        case MSGTYPE_SPACE_UPDATE_NEW_LIGHTSOURCE: {
            CameraWorkerSpaceUpdate_NewLightSource details;
            if (read_safely(fd_read, &details, sizeof(CameraWorkerSpaceUpdate_NewLightSource)) <= 0) {
                perror("read details of NewLightSource update");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }
            LightSource *entity = create_light_source(details.coord[0], details.coord[1], details.coord[2], 
                                              details.intensity);
            add_light_to_entity_space(space, entity, details.entity_id);
            break;
        }

        case MSGTYPE_SPACE_UPDATE_DELETE_ENTITY: {
            CameraWorkerSpaceUpdate_DeleteEntity details;
            if (read_safely(fd_read, &details, sizeof(CameraWorkerSpaceUpdate_DeleteEntity)) <= 0) {
                perror("read details of DeleteEntity update");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }
            delete_from_entity_space(space, details.entity_id);
            break;
        }

        case MSGTYPE_SPACE_UPDATE_DELETE_LIGHTSOURCE: {
            CameraWorkerSpaceUpdate_DeleteLightSource details;
            if (read_safely(fd_read, &details, sizeof(CameraWorkerSpaceUpdate_DeleteLightSource)) <= 0) {
                perror("read details of DeleteLightSource update");
                cleanup_child(fd_read, fd_write, space);
                exit(1);
            }
            delete_light_from_entity_space(space, details.entity_id);
            break;
        }

        default:
            fprintf(stderr,
                    "Unhandled message type '%d' received by worker at index %d.\n",
                    header.message_type, worker_idx);
            cleanup_child(fd_read, fd_write, space);
            exit(1);

        }
    }
    exit(0);
}


void spawn_single_camera_worker(pid_t *worker_pids,
                          int *pipe_read_fds,
                          int *pipe_write_fds,
                          int index,
                          EntitySpace *space) {

    int parent_to_child_pipe[2];
    int child_to_parent_pipe[2];

    if (pipe(parent_to_child_pipe) < 0) {
        perror("pipe - spawn child");
        exit(1);
    }
    if (pipe(child_to_parent_pipe) < 0) {
        perror("pipe - spawn child");
        exit(1);
    }

    worker_pids[index] = fork();
    if (worker_pids[index] == -1) {
        perror("fork");
        exit(1);
    }
    if (worker_pids[index] == 0) {
        // child
        
        fd_read_for_sig_handler = parent_to_child_pipe[0];
        fd_write_for_sig_handler = child_to_parent_pipe[1];
        space_for_sig_handler = space;

        if (signal(SIGINT, cleanup_child_sig) == SIG_ERR ||
            signal(SIGTERM, cleanup_child_sig) == SIG_ERR ||
            signal(SIGHUP, cleanup_child_sig) == SIG_ERR ||
            signal(SIGTSTP, cleanup_child_sig) == SIG_ERR) {
            
            perror("signal");
            exit(1);
        }


        close(parent_to_child_pipe[1]);
        close(child_to_parent_pipe[0]);
        camera_worker_work(
            index, parent_to_child_pipe[0], child_to_parent_pipe[1], space);
        // Should exit from inside this function (never return)
        // but just in case
        fprintf(stderr, "child escaped work");
        exit(1);
    }

    close(child_to_parent_pipe[1]);
    close(parent_to_child_pipe[0]);
    pipe_read_fds[index] = child_to_parent_pipe[0];
    pipe_write_fds[index] = parent_to_child_pipe[1];
}

void spawn_camera_workers(pid_t *worker_pids,
                          int *pipe_read_fds,
                          int *pipe_write_fds,
                          int count,
                          EntitySpace *space) {
    for (int i = 0; i < count; i++) {
        spawn_single_camera_worker(
            worker_pids, pipe_read_fds, pipe_write_fds, i, space);
    }
}

/**
 * Given an index,
 * - kill the worker at that index in worker_pids,
 * - close both pipes between the parent & worker
 * 
 * and then
 * - spawn a new worker
 * - put the new worker's pid & pipe fds into the arrays at that same index
 */
void respawn_single_worker_at_index(pid_t *worker_pids,
                                    int *worker_read_fds,
                                    int *worker_write_fds,
                                    int index) {
    // Kill the worker and reap it so it doesn't stay zombified for too long
    kill(worker_pids[index], SIGTERM);
    waitpid(worker_pids[index], NULL, 0);

    // spawn a new child
    fprintf(stderr, "Child at index %d is unresponsive. Respawning.\n", index);
    close(worker_read_fds[index]);
    close(worker_write_fds[index]);
    spawn_single_camera_worker(
        worker_pids, worker_read_fds, worker_write_fds, index, get_space());
}

/**
 * Given a list of `Entity`s, capture an image where each sample in the image
 * is the distance from the camera to the nearest collision in a particular
 * direction.
 *
 * - Arguments may not be NULL.
 * - The width and height of the image will be exactly those specified as part
 *   of the provided DistanceMap.
 * - All angles must be provided in radians.
 * - The horizontal_view_angle denotes how wide of an arc the camera's view
 *   frustum covers.
 *   More precisely, the leftmost column of pixels returned will be from rays
 *   shot in the azimuthal direction
 *           camera_azimuth + (1/2)(horizontal_view_angle),
 *   and the rightmost column of pixels will be from rays shot in the
 *   azimuthal direction
 *           camera_azimuth - (1/2)(horizontal_view_angle).
 * - The pixel_aspect_ratio is the ratio of (pixel width / pixel height) that
 *   the pixels will be displayed at. That is, if each pixel will be displayed
 *   twice as tall as it is wide, then pixel_aspect_ratio is 0.5.
 *   If the caller intends to display the camera's image with square pixels,
 *   pixel_aspect_ratio should be 1. This parameter is used to compensate for
 *   nonsquare pixel displays to prevent the image from looking stretched.
 */
void capture_image(
        DistanceMap *film,
        double horizontal_view_angle,
        double pixel_aspect_ratio,
        double camera_x,
        double camera_y,
        double camera_z,
        double camera_forward_azimuth,
        double camera_forward_inclination,
        int *worker_pids,
        int *worker_read_fds,
        int *worker_write_fds,
        int num_workers) {
    if(film == NULL) {
        fprintf(stderr, "must provide nonnull distance map");
        exit(1);
    }

    // find the three unit vectors that define the axes of the camera
    // definitely should not compute this on every frame
    // but it is here for now
    double camera_forward[3] = {
        cos(camera_forward_azimuth) * sin(camera_forward_inclination),
        sin(camera_forward_azimuth) * sin(camera_forward_inclination),
        cos(camera_forward_inclination)
    };
    double camera_upward[3] = {
        cos(camera_forward_azimuth) * sin(camera_forward_inclination - PI/2),
        sin(camera_forward_azimuth) * sin(camera_forward_inclination - PI/2),
        cos(camera_forward_inclination - PI/2)
    };
    double camera_rightward[3] = {
        camera_forward[1]*camera_upward[2]-camera_forward[2]*camera_upward[1],
        camera_forward[2]*camera_upward[0]-camera_forward[0]*camera_upward[2],
        camera_forward[0]*camera_upward[1]-camera_forward[1]*camera_upward[0]
    };


    int num_tasks = film->width * film->height;
    CameraRaycastTask task_list[num_tasks];
    int task_list_head = 0;
    int task_list_tail = 0;

    // calculate what the vertical and horizontal view angles should be
    double ax = horizontal_view_angle;
    double ay =
        (1 / pixel_aspect_ratio) *
        ((double) film->height / film->width) *
        (ax);

    // calculate the step angles between each ray
    double dx = ax / film->width;
    double dy = ay / film->height;


    // set the starting angles
    double theta_y = (ay / 2);  // radians from Forward towards Upward

    for(int ry = 0; ry < film->height; ry++, theta_y -= dy) {
        // start from the top row, move down
        double theta_x = -(ax / 2); // radians from Forward towards Rightward
        for(int rx = 0; rx < film->width; rx++, theta_x += dx) {
            // start from the leftmost column, move right
            CameraRaycastTask task;

            double rightward_coefficient = tan(theta_x);
            double upward_coefficient= tan(theta_y);
            // printf("%f, %f\n", theta_x, theta_y);

            task.image_x = rx;
            task.image_y = ry;
            task.ray_origin_x = camera_x;
            task.ray_origin_y = camera_y;
            task.ray_origin_z = camera_z;
            task.ray_direction_x =
                camera_forward[0] +
                camera_rightward[0] * rightward_coefficient +
                camera_upward[0] * upward_coefficient;
            task.ray_direction_y =
                camera_forward[1] +
                camera_rightward[1] * rightward_coefficient +
                camera_upward[1] * upward_coefficient;
            task.ray_direction_z =
                camera_forward[2] +
                camera_rightward[2] * rightward_coefficient +
                camera_upward[2] * upward_coefficient;

            task_list[task_list_tail] = task;

            task_list_tail++;
        }
    }

    // timing
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    int tasks_assigned = 0;
    int tasks_completed = 0;
    CameraRaycastTaskResult task_result; // repeatedly overwritten

    
    /////////////////////////////////
    // assign tasks to the workers //
    
    int max_read_fd, max_write_fd;
    fd_set select_read_fds;
    fd_set select_write_fds;
    FD_ZERO(&select_read_fds);
    FD_ZERO(&select_write_fds);
    max_read_fd = -1;
    max_write_fd = -1;

    // we can send the same header with each task.
    CameraMessageHeader task_header;
    task_header.message_type = MSGTYPE_RAYCAST_TASK;

    // send out an initial batch of tasks -- one each
    for (int i = 0; i < num_workers; i++) {
       
        // write the header that indicates an incoming task
        int write_result = write_safely(
            worker_write_fds[i], &task_header, sizeof(task_header));
        if (write_result < 0) {
            if(write_result == SENTINEL_WRITE_SAFELY_BROKEN_PIPE) {
                respawn_single_worker_at_index(
                    worker_pids, worker_read_fds, worker_write_fds, i);
            } else {
                perror("write - broadcast");
                exit(1);
            }
        }
       
        // write the actual task itself to the pipe
        write_result = write_safely(worker_write_fds[i],
                                    &task_list[task_list_head],
                                    sizeof(task_list[task_list_head]));
        if (write_result < 0) {
            if (write_result == SENTINEL_WRITE_SAFELY_BROKEN_PIPE) {
                respawn_single_worker_at_index(
                    worker_pids, worker_read_fds, worker_write_fds, i);
            } else {
                perror("write - broadcast");
                exit(1);
            }
        }

        // sleep(1);
        // printf("sent task to worker at index %d\n", i);
        task_list_head++;
        tasks_assigned++;
    }

    while(tasks_completed < num_tasks) {
        // since select_*_fds gets modified by select,
        // we need to do this on every loop iteration
        for (int i = 0; i < num_workers; i++) {
            FD_SET(worker_read_fds[i], &select_read_fds);
            if(worker_read_fds[i] >= max_read_fd) {
                max_read_fd = worker_read_fds[i] + 1;
            }
            FD_SET(worker_write_fds[i], &select_write_fds);
            if(worker_write_fds[i] >= max_write_fd) {
                max_write_fd = worker_write_fds[i] + 1;
            }
        }

        if(select(fmax(max_write_fd, max_read_fd),
                    &select_read_fds,
                    &select_write_fds, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        for (int i = 0; i < num_workers; i++) {
            if(FD_ISSET(worker_read_fds[i], &select_read_fds) != 0) {
                // THIS PIPE HAS SOMETHING FOR US TO READ

                int read_result =
                    read_safely(worker_read_fds[i],
                                &task_result,
                                sizeof(CameraRaycastTaskResult)) < 0;
                if (read_result < 0) {
                    perror("read");
                    exit(1);
                }

                int film_idx =
                    (film->width * task_result.image_y)
                    + task_result.image_x;
                film->distances[film_idx] = task_result.distance;
                tasks_completed++;

                if(FD_ISSET(worker_write_fds[i], &select_write_fds) != 0) {
                    // THIS PIPE IS AVAILABLE FOR WRITING TO

                    if (task_list_head >= num_tasks) {
                        // if there are no more tasks to write
                        break;
                    }

                    int write_result;
                    // write a header to indicate that a task is incoming
                    write_result = write_safely(
                        worker_write_fds[i], &task_header, sizeof(task_header));
                    if (write_result < 0) {
                        if (write_result == SENTINEL_WRITE_SAFELY_BROKEN_PIPE) {
                            respawn_single_worker_at_index(worker_pids,
                                                           worker_read_fds,
                                                           worker_write_fds,
                                                           i);
                            write_result = write_safely(worker_write_fds[i],
                                                        &task_header,
                                                        sizeof(task_header));
                            if (write_result < 0) {
                                perror("write - newly spawned worker already defective");
                                exit(1);
                            }
                        } else {
                            perror("write");
                            exit(1);
                        }
                    }

                    // write the task itself
                    write_result =
                        write_safely(worker_write_fds[i],
                                     &task_list[task_list_head],
                                     sizeof(task_list[task_list_head]));
                    if (write_result < 0) {
                        if (write_result == SENTINEL_WRITE_SAFELY_BROKEN_PIPE) {
                            respawn_single_worker_at_index(worker_pids,
                                                           worker_read_fds,
                                                           worker_write_fds,
                                                           i);
                            write_result = write_safely(worker_write_fds[i],
                                                        &task_header,
                                                        sizeof(task_header));
                            if (write_result < 0) {
                                perror("write - newly spawned worker already defective");
                                exit(1);
                            }
                        } else {
                            perror("write");
                            exit(1);
                        }
                    }

                    task_list_head++;
                    tasks_assigned++;
                }
 
            }
        }
    }

    // end time
    gettimeofday(&stop, NULL);
    // printf("took %lu microseconds. exiting with %d tasks completed\n",
    //         (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec,
    //         tasks_completed);
}

