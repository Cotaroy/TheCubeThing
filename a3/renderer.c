#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "renderer.h"
#include "raycast.h"

// there are 69 chars here not including space or null terminator
static char *hierarchy = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'."; // lightest to darkest

char get_pixel_char(double distance) {
    if (distance < EPSILON) {
        return hierarchy[0];
    }
    if(distance > MAX_DIST) {
        distance = MAX_DIST;
    }
    int num_tiers = 69;
    
    double relative_distance = (distance - MIN_DIST) / (MAX_DIST - MIN_DIST);

    int tier = (int) (sqrt(relative_distance) * (num_tiers - 1));

    // printf("%f corresponds to character '%c', with tier: %d\n", distance, hierarchy[tier], tier);
    return hierarchy[tier];
}

void render(DistanceMap *map) {
    int width = map->width;
    int height = map->height;
    double *distances = map->distances;

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            int idx = y * width + x;
            double dist = distances[idx];

            printf("%c", get_pixel_char(dist));
        }
        printf("\n");
    }
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
//             printf("%.1lf ", dist);
//
//             // if(dist > 32) printf(" ");
//             // else if(dist > 4) printf("*");
//             // else if(dist > 0) printf("#");
//             // else printf("@");
//         }
//         printf("\n");
//     }
// }
