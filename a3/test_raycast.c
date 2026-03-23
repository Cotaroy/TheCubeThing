#include <stdio.h>
#include "space.h"
#include "raycast.h"
#define PI (3.14159265358979323846)

int main() {

  Vertex *vertices = NULL;
  double pos[3] = {0, 0, 0};

  // test single ray
  Entity *cube = create_rectangle(NULL, &vertices, -1.5, -1.5, 5, 3, 3, 3);
  double distance = shoot_ray(pos, 0, 0, cube);

  printf("Expected: 5.000000\n");
  printf("Actual: %f\n", distance);

  // test same ray but put a cube in front
  create_rectangle(&cube, &vertices, -1.5, -1.5, 3, 3, 3, 3);
 
  distance = shoot_ray(pos, 0, 0, cube);

  printf("Expected: 3.000000\n");
  printf("Actual: %f\n", distance);
  
  // test shoot in x direction
  distance = shoot_ray(pos, 0, PI/2, cube);
  
  printf("Expected: inf\n");
  printf("Actual: %f\n", distance);

  // test shoot ray backwards
  distance = shoot_ray(pos, 0, PI/2, cube);
  
  printf("Expected: inf\n");
  printf("Actual: %f\n", distance);


  Entity *cube2 = create_rectangle(NULL, &vertices, 1, 1, 1, 1, 1, 1);
  distance = shoot_ray(pos, PI/4, PI/4, cube2);

  printf("Expected: 2\n");
  printf("Actual: %f\n", distance);

  distance = shoot_ray(pos, PI/4, 0.9553166, cube2);

  printf("Expected: 1.73\n");
  printf("Actual: %f\n", distance);

  free_all_entities(cube2);
  free_all_entities(cube);
  free_all_vertices(vertices);
}
