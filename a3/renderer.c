#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "renderer.h"


static char *hierarchy = "@%#WMH*+=-:."; // lightest to darkest

char get_pixel_char(double distance) {
    int num_tiers = 12;
    double min_dist = 0.0;
    double max_dist = 128.0;
    double tier_step = (max_dist - min_dist) / num_tiers;

    if(distance > max_dist) {
        distance = max_dist;
    }
    int tier = (distance - min_dist) / tier_step;

    return hierarchy[tier];
}

// void render(DistanceMap *map) {
//     int width = map->width;
//     int height = map->height;
//     double *distances = map->distances;
//
//     for(int y = 0; y < height; y++) {
//         for(int x = 0; x < width; x++) {
//             int idx = y * width + x;
//             double dist = distances[idx];
//
//             printf("%c", get_pixel_char(dist));
//         }
//         printf("\n");
//     }
// }

void render(DistanceMap *map) {
    int width = map->width;
    int height = map->height;
    double *distances = map->distances;

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            int idx = y * width + x;
            double dist = distances[idx];

            printf("%.1lf ", dist);

            // if(dist > 32) printf(" ");
            // else if(dist > 4) printf("*");
            // else if(dist > 0) printf("#");
            // else printf("@");
        }
        printf("\n");
    }
}
