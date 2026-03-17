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

// helper for shoot ray, returns distance from triangle
// return -1 if no intersection
double get_distance(double *pos, double pitch, double yaw, Triangle *triangle) {

}

// Shoot a single ray in the pitch-yaw direction from pos, returns distance
// if exit status is 0, then process terminated normally
double shoot_ray(double *pos, double pitch, double yaw, Entity *entities) {
  double x_component = cos(pitch) * sin(yaw);
  double y_component = sin(pitch) * sin(yaw);
  double z_component = cos(yaw);

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
      while (curr_tri != NULLL) {
        int res2 = fork();

        if (res2 == -1) {
          die("fork");
        }

        // child process
        else if (res == 0) {

          double distance = get_distance(pos, pitch, yaw, curr_tri);

          if (write(fd[1], &distance, sizeof(double)) == -1) {
            die("write");
          }

          if (close(fd[1]) == -1) {
            die("close");
          }

          exit(0);
        }
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
      }

      exit(0);
    }
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
  }

  double distance;
  double min_distance = INFINITY;

  // loop until we reach EOF
  while ((int read_res = read(fd[0], &distance, sizeof(double))) > 0) {
    if (read_res == -1) {
      die("read");
    }
    min_distance = fmin(min_distance, distance);
  }

  if (close(fd[0]) == -1) {
    die("close");
  }

  return min_distance;
}

int main() {

}
