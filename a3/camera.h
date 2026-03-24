#include "space.h"

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


