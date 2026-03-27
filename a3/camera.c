#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include "camera.h"
#include "raycast.h"
#include "renderer.h"
#include "space.h"
#define PI (3.14159265358979323846)


void normalise_vector_mutate(double *x, double *y, double *z) {
    double length = sqrt((*x)*(*x) + (*y)*(*y) + (*z)*(*z));
    *x /= length;
    *y /= length;
    *z /= length;
}


void camera_worker_work(
        int worker_idx,
        int fd_read,
        int fd_write,
        Entity *collidable_entities) {

    // overwritten repeatedly to store the current task & result
    CameraRaycastTask task;
    CameraRaycastTaskResult result;


    int tasks_completed = 0;
    int read_res;
    while((read_res = read(fd_read, &task, sizeof(CameraRaycastTask))) > 0) {
        double pos[3] = {
            task.ray_origin_x,
            task.ray_origin_y,
            task.ray_origin_z
        };
        // printf("parameters to shoot_ray: pos=(%lf,%lf,%lf),azim=%lf,incl=%lf",
        //         pos[0], pos[1], pos[2], task.ray_azimuth, task.ray_inclination);

        normalise_vector_mutate(
                &task.ray_direction_x,
                &task.ray_direction_y,
                &task.ray_direction_z
        );
        double distance = shoot_ray(
                pos,
                task.ray_direction_x,
                task.ray_direction_y,
                task.ray_direction_z,
                collidable_entities
        );

        // printf("[%d] Distance: %lf\n", worker_idx, distance);
        result.image_x = task.image_x;
        result.image_y = task.image_y;
        result.distance = distance;
        if(write(fd_write, &result, sizeof(result)) <= 0) {
            perror("child write");
            exit(1);
        }
        // printf("child wrote result for pixel (%d, %d)\n", result->image_x, result->image_y);

        tasks_completed++;
    }
    if (read_res == -1) {
        perror("read");
        exit(1);
    }
    if (close(fd_read) == -1) {
        perror("close");
        exit(1);
    }
    if (close(fd_write) == -1) {
        perror("close");
        exit(1);
    }
    // printf("Worker %d exiting after completing %d tasks.\n", worker_idx, tasks_completed);
    exit(0);
}


void spawn_camera_workers(
        pid_t *worker_pids,
        int *pipe_read_fds,
        int *pipe_write_fds,
        int count,
        Entity *collidable_entities) {

    int parent_to_child_pipe[2];
    int child_to_parent_pipe[2];

    for(int i = 0; i < count; i++) {
        if(pipe(parent_to_child_pipe) < 0) { perror("pipe"); exit(1); }
        if(pipe(child_to_parent_pipe) < 0) { perror("pipe"); exit(1); }

        worker_pids[i] = fork();
        if(worker_pids[i] == -1) {
            perror("fork");
            exit(1);
        }
        if(worker_pids[i] == 0) {
            // child

            close(parent_to_child_pipe[1]);
            close(child_to_parent_pipe[0]);
            camera_worker_work(i, parent_to_child_pipe[0],
                               child_to_parent_pipe[1],
                               collidable_entities);
            // Should exit from inside this function (never return)
            // but just in case
            fprintf(stderr, "child escaped work");
            exit(1);
        }

        close(child_to_parent_pipe[1]);
        close(parent_to_child_pipe[0]);
        pipe_read_fds[i] = child_to_parent_pipe[0];
        pipe_write_fds[i] = parent_to_child_pipe[1];
    }
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
        Entity *entities,
        DistanceMap *film,
        double horizontal_view_angle,
        double pixel_aspect_ratio,
        double camera_x,
        double camera_y,
        double camera_z,
        double camera_forward_azimuth,
        double camera_forward_inclination,
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
    CameraRaycastTask *task_list[num_tasks];
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


    // printf("======================================\n");
    // printf("Top left ray has θ=%lf, φ=%lf.\n", azim, incl);
    // printf("pxAR=%lf, a_azim=%lf, a_incl=%lf, d_azim=%lf, d_incl=%lf.\n",
    //         pixel_aspect_ratio,
    //         a_azim, a_incl,
    //         d_azim, d_incl);
    // printf("======================================\n");


    // set the starting angles
    double theta_y = (ay / 2);  // radians from Forward towards Upward

    for(int ry = 0; ry < film->height; ry++, theta_y -= dy) {
        // start from the top row, move down
        double theta_x = -(ax / 2); // radians from Forward towards Rightward
        for(int rx = 0; rx < film->width; rx++, theta_x += dx) {
            // start from the leftmost column, move right
            CameraRaycastTask *task = malloc(sizeof(CameraRaycastTask));
            if(task == NULL) {
                perror("malloc");
                exit(1);
            }

            double rightward_coefficient = tan(theta_x);
            double upward_coefficient= tan(theta_y);
            // printf("%f, %f\n", theta_x, theta_y);

            task->image_x = rx;
            task->image_y = ry;
            task->ray_origin_x = camera_x;
            task->ray_origin_y = camera_y;
            task->ray_origin_z = camera_z;
            task->ray_direction_x =
                camera_forward[0] +
                camera_rightward[0] * rightward_coefficient +
                camera_upward[0] * upward_coefficient;
            task->ray_direction_y =
                camera_forward[1] +
                camera_rightward[1] * rightward_coefficient +
                camera_upward[1] * upward_coefficient;
            task->ray_direction_z =
                camera_forward[2] +
                camera_rightward[2] * rightward_coefficient +
                camera_upward[2] * upward_coefficient;

            // printf("(%f, %f, %f)\n", camera_forward[0], camera_rightward[0], camera_upward[0]);
            // printf("(%f, %f, %f)\n", camera_forward[1], camera_rightward[1], camera_upward[1]);
            // printf("(%f, %f, %f)\n", camera_forward[2], camera_rightward[2], camera_upward[2]);
            // printf("[%d]: Shooting in direction (%f, %f, %f)\n", task_list_tail, task->ray_direction_x, task->ray_direction_y, task->ray_direction_z);

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


    int max_read_fd, max_write_fd;
    fd_set select_read_fds;
    fd_set select_write_fds;
    FD_ZERO(&select_read_fds);
    FD_ZERO(&select_write_fds);
    max_read_fd = -1;
    max_write_fd = -1;

    for (int i = 0; i < num_workers; i++) {
      if(write(worker_write_fds[i],
          task_list[task_list_head],
          sizeof(*task_list[task_list_head])) < 0) {
        perror("write");
        exit(1);
      }
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
                // this one has something to read from
                if(read(worker_read_fds[i], &task_result,
                            sizeof(CameraRaycastTaskResult)) <= 0) {
                    perror("read");
                    exit(1);
                }

                int film_idx =
                    (film->width * task_result.image_y)
                    + task_result.image_x;
                film->distances[film_idx] = task_result.distance;
                // printf("(%d, %d) Distance: %lf\n", task_result->image_x, task_result->image_y, task_result->distance);
                // printf("received result for pixel (%d, %d)\n", task_result->image_x, task_result->image_y);
                tasks_completed++;
                if(FD_ISSET(worker_write_fds[i], &select_write_fds) != 0) {
                // this one is available for writing to

                  if(task_list_head >= num_tasks) {
                    break;
                  }

                  if(write(worker_write_fds[i],
                            task_list[task_list_head],
                            sizeof(*task_list[task_list_head])) < 0) {
                    perror("write");
                    exit(1);
                  }
                // printf("sent task to worker at index %d\n", i);
                  task_list_head++;
                  tasks_assigned++;
                }
 
            }
        }
    }


    // end time
    gettimeofday(&stop, NULL);
    printf("took %lu microseconds. exiting with %d tasks completed\n",
            (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec,
            tasks_completed);

    for(int i = 0; i < num_tasks; i++) {
        // printf("[] %d\n", i);
        free(task_list[i]);
        task_list[i] = NULL;
    }
}

