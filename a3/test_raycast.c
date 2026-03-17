#include <stdio.h>
#include "space.h"
#include "raycast.h"
#define PI (3.14159265358979323846)

int main() {

  Vertex *vertices = NULL;
  Entity *cube = create_rectangle(NULL, &vertices, 5, -1.5, -1.5, 3, 3, 3);

  double pos[3] = {0, 0, 0};
  double distance = shoot_ray(pos, 0, PI, cube);

  printf("%f\n", distance);

  free_all_entities(cube);
  free_all_vertices(vertices);
}
