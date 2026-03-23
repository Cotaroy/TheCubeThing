#include <stdlib.h>
#include <stdio.h>
#include "space.h"
#include "raycast.h"
#include "renderer.h"
#define PI (3.14159265358979323846)

int main() {
    Vertex *vertices = NULL;
    Entity *cube = create_rectangle(NULL, &vertices, 5, -1.5, -1.5, 3, 3, 3);


    DistanceMap *map = malloc(sizeof(DistanceMap));
    map->width = 128;
    map->height = 128;
    map->distances = malloc(sizeof(double) * 128 * 128);
    double pos[3] = {0, 0, 0};
    double fov = PI / 2;
    int count = 0;
    for(int y = -64; y < 63; y++) {
        for(int x = -64; x < 63; x++) {
            // int idx = (y + 64) * 128 + (x + 64);
            double pitch = (fov/128) * y;
            double yaw = (fov/128) * x;
            double distance = shoot_ray(pos, pitch, yaw, cube);
            printf("%d: %f\n", count, distance);
            count++;
        }
    }

    // printf("%f\n", distance);

    render(map);
    free(map->distances);
    free(map);

    free_all_entities(cube);
    free_all_vertices(vertices);
}
