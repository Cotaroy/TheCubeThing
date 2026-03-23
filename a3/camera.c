#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include "camera.h"
#include "raycast.h"
#include "renderer.h"
#include "space.h"
#define PI (3.14159265358979323846)

#define NUM_WORKERS 8


void camera_worker_work(
        int worker_idx,
        int fd_read,
        int fd_write,
        Entity *collidable_entities) {
    CameraRaycastTask task;
    int tasks_completed = 0;
    while(read(fd_read, &task, sizeof(CameraRaycastTask)) > 0) {
        printf("worker %d actually received its %dth task\n", worker_idx, tasks_completed + 1);
        double pos[3] = {
            task.ray_origin_x,
            task.ray_origin_y,
            task.ray_origin_z
        };
        double distance = shoot_ray(pos,
                task.ray_azimuth,
                task.ray_inclination,
                collidable_entities);
        printf("[%d] Distance: %lf\n", worker_idx, distance);
        tasks_completed++;
    }
    printf("Worker %d exiting after completing %d tasks.\n", worker_idx, tasks_completed);
    exit(0);
}


void camera_spawn_workers(
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
        double camera_azimuth,
        double camera_inclination) {

    if(film == NULL) {
        fprintf(stderr, "must provide nonnull distance map");
        exit(1);
    }

    int num_workers = NUM_WORKERS;
    pid_t pids[num_workers];
    int read_fds[num_workers];
    int write_fds[num_workers];

    camera_spawn_workers(pids, read_fds, write_fds, num_workers, entities);


    int num_tasks = film->width * film->height;
    CameraRaycastTask *task_list[num_tasks];
    int task_list_head = 0;
    int task_list_tail = 0;

    // calculate what the vertical and horizontal view angles should be
    double a_azim = horizontal_view_angle;
    double a_incl =
        (1 / pixel_aspect_ratio) *
        (film->height / film->width) *
        (a_azim);

    // calculate the step angles between each ray
    double d_azim = a_azim / film->width;
    double d_incl = a_incl / film->height;

    // create all the tasks
    double azim = camera_azimuth + (a_azim / 2); // counterclockwise from camera
    double incl = camera_inclination - (a_incl / 2); // up from camera
    for(int y = 0; y < film->height; y++, incl += d_incl) {
        // start from the top row, move down
        for(int x = 0; x < film->width; x++, azim -= d_azim) {
            // start from the leftmost column, move right (clockwise; azim decr)
            CameraRaycastTask *task = malloc(sizeof(CameraRaycastTask));
            if(task == NULL) {
                perror("malloc");
                exit(1);
            }
            task->image_x = x;
            task->image_y = y;
            task->ray_origin_x = camera_x;
            task->ray_origin_y = camera_y;
            task->ray_origin_z = camera_z;
            task->ray_azimuth = azim;
            task->ray_inclination = incl;
            task_list[task_list_tail] = task;

            task_list_tail++;
        }
    }

    // timing
    struct timeval start, stop;
    gettimeofday(&start, NULL);

    int tasks_assigned = 0;
    int tasks_completed = 0;

    fd_set select_fds;
    int max_fd = -1;
    FD_ZERO(&select_fds);

    while(tasks_completed < num_tasks) {
        // since select_fds gets modified by select,
        // we need to do this on every loop iteration
        for (int i = 0; i < num_workers; i++) {
            FD_SET(write_fds[i], &select_fds);
            if(write_fds[i] >= max_fd) {
                max_fd = write_fds[i] + 1;
            }
        }

        if(select(max_fd, NULL, &select_fds, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        for (int i = 0; i < num_workers; i++) {
            if(FD_ISSET(write_fds[i], &select_fds) != 0) {
                // this one is available for writing to

                if(task_list_head >= num_tasks) {
                    break;
                }

                if(write(write_fds[i],
                            task_list[task_list_head],
                            sizeof(*task_list[task_list_head])) < 0) {
                    perror("write");
                    exit(1);
                }
                printf("sent task to worker at index %d\n", i);
                task_list_head++;
                tasks_assigned++;
            }
        }
        // if(available_write_fd == -1) {
        //     perror("select returned but no pipes were available to write??");
        //     exit(1);
        // }

        if(task_list_head >= num_tasks) {
            break;
        }
    }

    // end time
    gettimeofday(&stop, NULL);
    printf("took %lu microseconds\n", (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec);

    for(int i = 0; i < num_tasks; i++) {
        // printf("[] %d\n", i);
        free(task_list[i]);
    }
}



int main() {
    Vertex *vertices = NULL;
    Entity *cube = create_rectangle(NULL, &vertices, 0, 8, 0, 3, 3, 3);
    create_rectangle(&cube, &vertices, 0, 8, 0, 3, 3, 3);
    create_rectangle(&cube, &vertices, 0, 8, 0, 3, 3, 3);
    create_rectangle(&cube, &vertices, 0, 8, 0, 3, 3, 3);
    create_rectangle(&cube, &vertices, 0, 8, 0, 3, 3, 3);


    DistanceMap *map = malloc(sizeof(DistanceMap));
    map->width = 128;
    map->height = 128;
    map->distances = malloc(sizeof(double) * 128 * 128);

    capture_image(cube, map, 0,0,0,0,0,0,0);
    return 0;
}












