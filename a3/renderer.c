#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ansi_escape_sequences.h"
#include "renderer.h"
#include "raycast.h"

// there are 69 chars here not including space or null terminator
// lightest to darkest
static char *hierarchy = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'.";

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

char get_pixel_char_light(double luminosity) {
    if (luminosity < MIN_LIGHT) {
        luminosity = MIN_LIGHT;
    }
    else if(luminosity > MAX_LIGHT) {
        luminosity = MAX_LIGHT;
    }
    double num_tiers = 69.;

    double tier_step = (MAX_LIGHT - MIN_LIGHT) / (num_tiers + 1);
    int tier = luminosity / tier_step;
    if (68 - tier < 0 ) {
        return hierarchy[0];
    }
    // printf("Returning with tier %d = %f / (10/7)\n", 68 - tier, luminosity);

    // printf("%f corresponds to character '%c', with tier: %d\n", distance, hierarchy[tier], tier);
    return hierarchy[68 - tier];
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

void render_luminosity(DistanceMap *map) {
    int width = map->width;
    int height = map->height;
    double *luminosity = map->distances;

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            int idx = y * width + x;
            double lum = luminosity[idx];

            printf("%c", get_pixel_char_light(lum));
        }
        printf("\n");
    }
}

