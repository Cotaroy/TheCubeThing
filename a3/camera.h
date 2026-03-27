#ifndef CAMERA_H
#define CAMERA_H

#include <stdint.h>
#include "space.h"
#include "renderer.h"

#define MSGTYPE_RAYCAST_TASK 0x00
#define MSGTYPE_SPACE_UPDATE_TRANSLATE_ENTITY 0x10
#define MSGTYPE_SPACE_UPDATE_ROTATE_ENTITY 0x11

typedef struct {
    uint8_t message_type;
    uint32_t length; // number of bytes that come after this header
                     // only used for variable-width message types
} CameraMessageHeader;

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

typedef struct {
    int entity_id;
    double x_offset;
    double y_offset;
    double z_offset;
} CameraWorkerSpaceUpdate_TranslateEntity;

void capture_image(
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
    EntitySpace *space);

ssize_t write_safely(int destination_file_descriptor, void *source_buffer,
                     size_t num_bytes_wanted);

#endif
