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

Triangle *create_triangle(double *v1, double *v2, double *v3);
Triangle *create_triangle_v(Vertex *v1, Vertex *v2, Vertex *v3);
Entity *create_entity(Triangle *object);
Vertex *create_vertex(double *coordinate);

// (x,y,z) is the coordinate of the front left bottom vertex of the rectangle
Entity *create_rectangle(Entity *entities, Vertex **vertex_list, double x, double y, double z, double x_length, double y_length, double z_length);
void free_all_triangles(Triangle *triangles);
void free_all_entities(Entity *entities);
void free_all_vertices(Vertex *vertices);

#endif
