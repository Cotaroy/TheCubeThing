#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>
#include "space.h"
#include "renderer.h"
#include "camera.h"
#define PI (3.14159265358979323846)

int main() {
    Vertex *vertices = NULL;
    // Entity *cube =
    //     create_rectangle(NULL, &vertices, -1.5, -1.5, 8, 3, 3, 3);
    // create_rectangle(&cube, &vertices, -10, -10, 20, 20, 20, 1);
    // create_rectangle(&cube, &vertices, -100, -100, 100, 200, 200, 1);

    Entity *cube =
        create_rectangle(NULL, &vertices, -100, -100, 100, 200, 200, 1);
    create_rectangle(&cube, &vertices, -10, -10, 20, 20, 20, 1);
    create_rectangle(&cube, &vertices, -1.5, -1.5, 1, 3, 3, 3);

    DistanceMap *map = malloc(sizeof(DistanceMap));
    map->width = 235;
    map->height = 70;
    map->distances = malloc(sizeof(double) * map->width * map->height);
    for(int i = 0; i < map->width * map->height; i++) {
        map->distances[i] = -42;
    }

    capture_image(cube, map, PI/4, 11./21., 0, 0, -20, 0, 0);
    render(map);

    free_all_entities(cube);
    free_all_vertices(vertices);

    return 0;
}


