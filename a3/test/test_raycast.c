#include <stdio.h>
#include <math.h>
#include "../space.h"
#include "../raycast.h"
#define PI (3.14159265358979323846)

int main() {

    printf("Testing shoot_ray:\n");
    double pos[3] = {0, 0, 0};

    EntitySpace *space = create_space();
    // test single ray
    Entity *cube = create_rectangle(-1.5, -1.5, 5, 3, 3, 3);

    add_to_entity_space(space, cube, 0);

    double x = cos(0) * sin(0);
    double y = sin(0) * sin(0);
    double z = cos(0);

    double distance = shoot_ray(pos, x, y, z, space);

    printf("Expected: 5.000000\n");
    printf("Actual: %f\n", distance);

    // test same ray but put a cube in front
    Entity *cube1 = create_rectangle(-1.5, -1.5, 3, 3, 3, 3);
    
    add_to_entity_space(space, cube1, 1);

    distance = shoot_ray(pos, x, y, z, space);

    printf("Expected: 3.000000\n");
    printf("Actual: %f\n", distance);

    x = cos(0) * sin(PI/2);
    y = sin(0) * sin(PI/2);
    z = cos(PI/2);
    // test shoot in x direction
    distance = shoot_ray(pos, x, y, z, space);

    printf("Expected: inf\n");
    printf("Actual: %f\n", distance);

    // test shoot ray backwards
    distance = shoot_ray(pos, x, y, z, space);

    printf("Expected: inf\n");
    printf("Actual: %f\n", distance);

    x = cos(PI/4) * sin(PI/4);
    y = sin(PI/4) * sin(PI/4);
    z = cos(PI/4);

    EntitySpace *space2 = create_space();
    Entity *cube2 = create_rectangle(1, 1, 1, 1, 1, 1);

    add_to_entity_space(space2, cube2, 0);

    distance = shoot_ray(pos, x, y, z, space2);

    printf("Expected: 2\n");
    printf("Actual: %f\n", distance);

    x = cos(PI/4) * sin(0.9553166);
    y = sin(PI/4) * sin(0.9553166);
    z = cos(0.9553166);

    distance = shoot_ray(pos, x, y, z, space2);

    printf("Expected: 1.73\n");
    printf("Actual: %f\n", distance);

    printf("\n\nTesting shoot_light_ray:\n");
    printf("Test 1: Light blocked by cube\nExpected: 0.0\n");

    EntitySpace *space3 = create_space();
    Entity *cube3 = create_rectangle(-.5, 2, -.5, 1, 1, 1);

    add_to_entity_space(space3, cube3, 0);
    LightSource *light = create_light_source(0, 3, 0, 500);
    add_light_to_entity_space(space3, light, 0);
    double intensity = shoot_light_ray(pos, 0, 1, 0, space3);

    printf("Actual: %.1f\n", intensity);

    free_space(space);
    free_space(space2);
    free_space(space3);
}
