#ifndef RENDERER_H
#define RENDERER_H
#define MAX_DIST 128.0
#define MIN_DIST 0.0
#define EPSILON 1e-8

typedef struct {
    int width;
    int height;
    double* distances; // array of length width*height, stored in LTR,TTB reading direction
} DistanceMap;

// typedef struct {
//     void (*render)(DistanceMap map);
// } Renderer;

void render(DistanceMap *map);

#endif
