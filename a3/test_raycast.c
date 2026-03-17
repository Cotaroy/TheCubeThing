#include <stdio.h>
#include "space.h"
#include "raycast.h"
#define PI (3.14159265358979323846)

int main() {

  Vertex *vertices = NULL;
  Entity *cube = create_rectangle(NULL, &vertices, 5, -1.5, -1.5, 3, 3, 3);

  double pos[3] = {0, 0, 0};
  double distance = shoot_ray(pos, 0, PI/2, cube);

  printf("Expected: 5.000000\n");
  printf("Actual: %f\n", distance);

  create_rectangle(&cube, &vertices, 3, 0, 0, 1, 1, 1);
 
  distance = shoot_ray(pos, 0, PI/2, cube);

  printf("Expected: 3.000000\n");
  printf("Actual: %f\n", distance);

  free_all_entities(cube);
  free_all_vertices(vertices);
}
