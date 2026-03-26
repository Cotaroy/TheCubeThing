#ifndef RENDERER_H
#define RENDERER_H

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
