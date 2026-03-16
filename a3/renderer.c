#include <stdlib.h>
#include <stdio.h>
#include "renderer.h"

void render(DistanceMap *map) {
    int width = map->width;
    int height = map->height;
    double *distances = map->distances;

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            int idx = y*width + x;
            int dist = distances[idx];
            
            if(dist > 0.5) {
                printf(" ");
            }
            else if(dist > 0.25) {
                printf(".");
            }
            else {
                printf("#");
            }
        }
        printf("\n");
    }
}

// testinggin
int main() {
    DistanceMap *map = malloc(sizeof(DistanceMap));
    map->width = 4;
    map->height = 4;
    
    double *distances = malloc(sizeof(double) * 16);
    for(int i = 0; i < 16; i++) {
        distances[i] = i/16.0;
    }
    map->distances = distances;

    render(map);

    free(map->distances);
    free(map);

    return 0;
}
