#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>
#include "raycast.h"
#define die(e) do { perror(e); exit(EXIT_FAILURE); } while (0);

Camera *create_camera(double *pos, double pitch, double yaw) {
  Camera *new_camera = malloc(sizeof(Camera));

  new_camera->position = pos;
  new_camera->pitch = pitch;
  new_camera->yaw = yaw;

  return new_camera;
}

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
double get_distance(double *pos, double pitch, double yaw, Triangle *triangle) {

  double ray_vec[3];
  ray_vec[0] = cos(pitch) * sin(yaw);
  ray_vec[1] = sin(pitch) * sin(yaw);
  ray_vec[2] = cos(yaw);

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

  // check if ray_vec orthogonal to normal
  if (fabs(dot_product(ray_vec, plane_normal)) < ORTHOGONAL_TOLERANCE) {
    return INFINITY;
  }

  // find intersection with plane
  double plane_offset = -1 * dot_product(plane_normal, triangle->vertex0);

  double distance = -1 * (dot_product(plane_normal, pos) + plane_offset) / (dot_product(plane_normal, ray_vec));

  // if intersection is behind the source
  if (distance < 0) {
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

  double unit_normal[3];
  normalize(unit_normal, plane_normal);

  // actual logic for checking
  if (dot_product(cross1, unit_normal) >= 0 && dot_product(cross2, unit_normal) >= 0 && dot_product(cross3, unit_normal) >= 0) {
    return distance;
  }
  return INFINITY;

}

// Shoot a single ray in the pitch-yaw direction from pos, returns distance
// if exit status is 0, then process terminated normally
double shoot_ray(double *pos, double pitch, double yaw, Entity *entities) {

  int fd[2];
  if (pipe(fd) <= -1) {
    die("pipe");
  }

  Entity *curr = entities;

  // fork for every Entity
  while (curr != NULL) {
    int res = fork();

    // failure check
    if (res == -1) {
      die("fork");
    }

    // child process
    else if (res == 0) {
      
      // close read end
      if (close(fd[0]) == -1) {
        die("close");
      }

      Triangle *curr_tri = curr->object;

      // fork for every Triangle
      while (curr_tri != NULL) {
        int res2 = fork();

        if (res2 == -1) {
          die("fork");
        }

        // child process
        else if (res2 == 0) {

          double distance = get_distance(pos, pitch, yaw, curr_tri);

          if (write(fd[1], &distance, sizeof(double)) == -1) {
            die("write");
          }

          if (close(fd[1]) == -1) {
            die("close");
          }

          exit(0);
        }
        curr_tri = curr_tri->next;
      }

      // only the parent will finish the loop, it should only need to wait
      
      if (close(fd[1]) == -1) {
        die("close");
      }

      curr_tri = curr->object;
      while (curr_tri != NULL) {

        int status;
        if (wait(&status) == -1) {
          die("wait");
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
          fprintf(stderr, "Something went wrong in triangle fork\n");
          exit(1);
        }
        curr_tri = curr_tri->next;
      }

      exit(0);
    }
    curr = curr->next;
  }

  // only the parent will finish the loop
  // close write end
  if (close(fd[1]) == -1) {
    die("close");
  }

  curr = entities;
  while (curr != NULL) {
    
    int status;
    if (wait(&status) == -1) {
      die("wait");
    }
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
      fprintf(stderr, "Something went wrong in entity fork\n");
      exit(1);
    }
    curr = curr->next;
  }

  double distance;
  double min_distance = INFINITY;

  // loop until we reach EOF
  int read_res;
  do {
    read_res = read(fd[0], &distance, sizeof(double));
    if (read_res == -1) {
      die("read");
    }
    
    min_distance = fmin(min_distance, distance);
  } while (read_res > 0);

  if (close(fd[0]) == -1) {
    die("close");
  }

  return min_distance;
}
