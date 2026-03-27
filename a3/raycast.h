#ifndef RAYCAST_H
#define RAYCAST_H
#include "space.h"
#define ORTHOGONAL_TOLERANCE 1e-5

void cross_product(double *result, double *v1, double *v2); // expects array of length 3
double dot_product(double *v1, double *v2); // expects array of length 3
void normalize(double *result, double *vector);

// assume that x,y,z vectors are normalized
// return INFINITY if there is no intersection or if intersection is behind the source
double get_distance(double *pos, double x_vector, double y_vector, double z_vector, Triangle *triangle);
// return INFINITY if there is no intersection or if intersection is behind the source
double shoot_ray(double *pos, double x_vector, double y_vector, double z_vector, EntitySpace *space);

#endif
