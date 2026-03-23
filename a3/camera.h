#include "space.h"

typedef struct {
    int image_x;
    int image_y;
    double ray_origin_x;
    double ray_origin_y;
    double ray_origin_z;
    double ray_azimuth;
    double ray_inclination;
} CameraRaycastTask;

typedef struct {
    int image_x;
    int image_y;
    double distance;
} CameraRaycastTaskResult;


