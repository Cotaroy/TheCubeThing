#ifndef CAMERA_H
#define CAMERA_H

#include "space.h"
#include "renderer.h"

typedef struct {
    int image_x;
    int image_y;
    double ray_origin_x;
    double ray_origin_y;
    double ray_origin_z;
    double ray_direction_x; // not necessarily normalised
    double ray_direction_y;
    double ray_direction_z;
} CameraRaycastTask;

typedef struct {
    int image_x;
    int image_y;
    double distance;
} CameraRaycastTaskResult;

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
    int num_workers);

void spawn_camera_workers(
    pid_t *worker_pids,
    int *pipe_read_fds,
    int *pipe_write_fds,
    int count,
    Entity *collidable_entities);
        
#endif
