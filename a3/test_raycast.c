#include <stdio.h>
#include <math.h>
#include "space.h"
#include "raycast.h"
#define PI (3.14159265358979323846)

int main() {

  double pos[3] = {0, 0, 0};

  // test single ray
  Entity *cube = create_rectangle(NULL, -1.5, -1.5, 5, 3, 3, 3);

  double x = cos(0) * sin(0);
  double y = sin(0) * sin(0);
  double z = cos(0);

  double distance = shoot_ray(pos, x, y, z, cube);

  printf("Expected: 5.000000\n");
  printf("Actual: %f\n", distance);

  // test same ray but put a cube in front
  Entity *cube1 = create_rectangle(cube, -1.5, -1.5, 3, 3, 3, 3);
 
  distance = shoot_ray(pos, x, y, z, cube);

  printf("Expected: 3.000000\n");
  printf("Actual: %f\n", distance);
  
  x = cos(0) * sin(PI/2);
  y = sin(0) * sin(PI/2);
  z = cos(PI/2);
  // test shoot in x direction
  distance = shoot_ray(pos, x, y, z, cube);
  
  printf("Expected: inf\n");
  printf("Actual: %f\n", distance);

  // test shoot ray backwards
  distance = shoot_ray(pos, x, y, z, cube);
  
  printf("Expected: inf\n");
  printf("Actual: %f\n", distance);

  x = cos(PI/4) * sin(PI/4);
  y = sin(PI/4) * sin(PI/4);
  z = cos(PI/4);

  Entity *cube2 = create_rectangle(NULL, 1, 1, 1, 1, 1, 1);
  distance = shoot_ray(pos, x, y, z, cube2);

  printf("Expected: 2\n");
  printf("Actual: %f\n", distance);

  x = cos(PI/4) * sin(0.9553166);
  y = sin(PI/4) * sin(0.9553166);
  z = cos(0.9553166);

  distance = shoot_ray(pos, x, y, z, cube2);

  printf("Expected: 1.73\n");
  printf("Actual: %f\n", distance);

  free_all_entities(cube2);
  free_all_entities(cube1);
}
