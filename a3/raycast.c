#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include "raycast.h"
#define die(e) do { perror(e); exit(EXIT_FAILURE); } while (0);

void cross_product(double *result, double *v1, double *v2) {
  
  result[0] = v1[1] * v2[2] - v1[2] * v2[1];
  result[1] = v1[2] * v2[0] - v1[0] * v2[2];
  result[2] = v1[0] * v2[1] - v1[1] * v2[0];

}

double dot_product(double *v1, double *v2) {
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void normalize(double *result, double *vector) {
  double norm = sqrt(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);

  for (int i = 0; i < 3; i++) {
    result[i] = vector[i] / norm;
  }
}

// helper for shoot ray, returns distance from triangle
// return INFINITY if no intersection
double get_distance(double *pos, double x_vector, double y_vector, double z_vector, Triangle *triangle, double *intersection) {

  double ray_vec[3];
  ray_vec[0] = x_vector;
  ray_vec[1] = y_vector;
  ray_vec[2] = z_vector;

  // 2 vectors to define plane
  double p_vector1[3];
  double p_vector2[3];
  // third one for later
  double p_vector3[3];

  for (int i = 0; i < 3; i++) {
    p_vector1[i] = triangle->vertex1[i] - triangle->vertex0[i];
    p_vector2[i] = triangle->vertex2[i] - triangle->vertex1[i];
    p_vector3[i] = triangle->vertex0[i] - triangle->vertex2[i];
  }

  // get cross product
  double plane_normal[3];
  cross_product(plane_normal, p_vector1, p_vector2);
  double unit_normal[3];
  normalize(unit_normal, plane_normal);

  // check if ray_vec orthogonal to normal
  if (fabs(dot_product(ray_vec, plane_normal)) < ORTHOGONAL_TOLERANCE) {
    if (intersection != NULL) {
        intersection[0] = INFINITY;
        intersection[1] = INFINITY;
        intersection[2] = INFINITY;
    }
    return INFINITY;
  }

  // find intersection with plane
  double plane_offset = -1 * dot_product(unit_normal, triangle->vertex0);

  double distance = -1 * (dot_product(unit_normal, pos) + plane_offset) / (dot_product(unit_normal, ray_vec));

  // if intersection is behind the source
  if (distance < ORTHOGONAL_TOLERANCE) {
    if (intersection != NULL) {
        intersection[0] = INFINITY;
        intersection[1] = INFINITY;
        intersection[2] = INFINITY;
    }
    return INFINITY;
  }

  double intersection_point[3];
  for (int i = 0; i < 3; i++) {
    intersection_point[i] = pos[i] + ray_vec[i] * distance;
  }

  // check if intersection is in triangle
  
  double intersect_minus1[3];
  double intersect_minus2[3];
  double intersect_minus3[3];

  for (int i = 0; i < 3; i++) {
    intersect_minus1[i] = intersection_point[i] - triangle->vertex0[i];
    intersect_minus2[i] = intersection_point[i] - triangle->vertex1[i];
    intersect_minus3[i] = intersection_point[i] - triangle->vertex2[i];
  }

  double cross1[3];
  double cross2[3];
  double cross3[3];

  cross_product(cross1, p_vector1, intersect_minus1);
  cross_product(cross2, p_vector2, intersect_minus2);
  cross_product(cross3, p_vector3, intersect_minus3);


  // actual logic for checking
  if (dot_product(cross1, unit_normal) >= -ORTHOGONAL_TOLERANCE && dot_product(cross2, unit_normal) >= -ORTHOGONAL_TOLERANCE && dot_product(cross3, unit_normal) >= -ORTHOGONAL_TOLERANCE) {
    if (intersection != NULL) {
        intersection[0] = intersection_point[0]; intersection[1] = intersection_point[1]; intersection[2] = intersection_point[2];
    }
    return distance;
  }

  if (intersection != NULL) {
    intersection[0] = INFINITY;
    intersection[1] = INFINITY;
    intersection[2] = INFINITY;
  }
  return INFINITY;

}

// Shoot a single ray in the x,y,z direction from pos, returns distance
// if exit status is 0, then process terminated normally
double shoot_ray(double *pos, double x_vector, double y_vector, double z_vector, EntitySpace *space) {
  
  double min_distance = INFINITY;

  for (int i = 0; i < MAX_ENTITIES; i++) {
    Triangle *curr_triangle = get_object(space, i);
    while (curr_triangle != NULL) {
      double distance = get_distance(pos, x_vector, y_vector, z_vector, curr_triangle, NULL);

      min_distance = fmin(min_distance, distance);

      curr_triangle = curr_triangle->next;
    }
  }

  return min_distance;
}

double shoot_light_ray(double *pos, double x_vector, double y_vector, double z_vector, EntitySpace *space) {

    double min_distance = INFINITY;
    double min_intersection_point[3] = {INFINITY, INFINITY, INFINITY};

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Triangle *curr_triangle = get_object(space, i);
        while (curr_triangle != NULL) {

            double intersection_point[3];

            double distance = get_distance(pos, x_vector, y_vector, z_vector, curr_triangle, intersection_point);

            if (min_distance > distance) {
                min_intersection_point[0] = intersection_point[0];
                min_intersection_point[1] = intersection_point[1];
                min_intersection_point[2] = intersection_point[2];
                min_distance = distance;
            }

            curr_triangle = curr_triangle->next;
        }
    }
    
    if (min_distance == INFINITY || min_intersection_point[0] == INFINITY) {
        return 0;
    }

    double total_intensity = 0;

    for (int i = 0; i < MAX_LIGHTS; i++) {
        LightSource *source = get_light(space, i);
        if (source == NULL) {
            continue;
        }

        double x_to_light = source->x - min_intersection_point[0];
        double y_to_light = source->y - min_intersection_point[1];
        double z_to_light = source->z - min_intersection_point[2];

        int obstructed = 0;

        for (int j = 0; j < MAX_ENTITIES && obstructed == 0; j++) {
            Triangle *curr_triangle = get_object(space, j);
            while (curr_triangle != NULL) {

                double distance = get_distance(pos, x_to_light, y_to_light, z_to_light, curr_triangle, NULL);

                if (distance != INFINITY) {
                    obstructed = 1;
                    break;
                }

                curr_triangle = curr_triangle->next;
            }
        }

        if (obstructed == 0) {
            double distance_squared = x_to_light * x_to_light + y_to_light * y_to_light + z_to_light * z_to_light;
            total_intensity += source->intensity / distance_squared;
        }
    }

    return total_intensity / (min_distance * min_distance);
}
