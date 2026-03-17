#ifndef RAYCAST_H
#define RAYCAST_H
#include "space.h"
#define ORTHOGONAL_TOLERANCE 1e-5

// pitch and yaw WILL BE IN RADIANS!!!
typedef struct camera {
  double *position;
  double pitch;
  double yaw;
} Camera;

Camera *create_camera(double *pos, double pitch, double yaw);
void cross_product(double *result, double *v1, double *v2); // expects array of length 3
double dot_product(double *v1, double *v2); // expects array of length 3
void normalize(double *result, double *vector);

// return INFINITY if there is no intersection or if intersection is behind the source
double get_distance(double *pos, double pitch, double yaw, Triangle *triangle);
// return INFINITY if there is no intersection or if intersection is behind the source
double shoot_ray(double *pos, double pitch, double yaw, Entity *entities);

#endif
