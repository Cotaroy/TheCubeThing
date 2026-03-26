#ifndef SPACE_H
#define SPACE_H

typedef struct triangle {
  double *vertex0;
  double *vertex1;
  double *vertex2;
  struct triangle *next; // triangle lists will be NULL terminated
} Triangle;

typedef struct entity {
  Triangle *object;
  struct entity *next; // entity list will be NULL terminated
} Entity;

// this struct is mainly for freeing vertices (fix double freeing problem if same vertex used twice)
typedef struct vertex {
  double *coordinate;
  struct vertex *next; // vertices will be NULL terminated
} Vertex;

Entity *create_entity(Triangle *object);

// (x,y,z) is the coordinate of the front left bottom vertex of the rectangle
Entity *create_rectangle(Entity **entities, Vertex **vertex_list, double x, double y, double z, double x_length, double y_length, double z_length);
void free_all_entities(Entity *entities);
void free_all_vertices(Vertex *vertices);

#endif
