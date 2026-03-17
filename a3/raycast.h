#ifndef RAYCAST_H
#define RAYCAST_H
#include "space.h"

// pitch and yaw WILL BE IN RADIANS!!!
typedef struct camera {
  int *position;
  int pitch;
  int yaw;
} Camera;

Camera *create_camera(double *pos, double pitch, double yaw);

// return -1 if there is no intersection
double get_distance(double *pos, double pitch, double yaw, Triangle *triangle);
double shoot_ray(double *pos, double pitch, double yaw, Entity *entities);

#endif
