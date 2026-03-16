#ifndef SPACE_H
#define SPACE_H

typedef struct triangle {
  int *vertex0;
  int *vertex1;
  int *vertex2;
  struct triangle *next; // triangle lists will be NULL terminated
} Triangle;

typedef struct entity {
  Triangle *object;
  struct entity *next; // entity list will be NULL terninated
} Entity;

Triangle *create_triangle(int *v1, int *v2, int *v3);
Entity *create_entity(Triangle *object);
// (x,y,z) is the coordinate of the front left bottom vertex of the rectangle
Entity *create_rectangle(Entity *entities, int x, int y, int z, int x_length, int y_length, int z_length);
void free_all_triangles(Triangle *triangles);
void free_all_entities(Entity *entities);

#endif
