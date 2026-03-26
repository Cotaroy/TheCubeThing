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
  double x_center;
  double y_center;
  double z_center;
  struct entity *next; // entity list will be NULL terminated
} Entity;

// this struct is mainly for freeing vertices (fix double freeing problem if same vertex used twice)
typedef struct vertex {
  double *coordinate;
  struct vertex *next; // vertices will be NULL terminated
} Vertex;

Entity *create_entity(Triangle *object);

// (x,y,z) is the coordinate of the front left bottom vertex of the rectangle
Entity *create_rectangle(Entity **entities, double x, double y, double z, double x_length, double y_length, double z_length);

// degree is the degree of rotation
// (x,y,z) is the position we are rotating around
void translate(Entity *entity, double x_offset, double y_offset, double z_offset);
void rotate_x(Entity *entity, double degree, double x, double y, double z);
void rotate_y(Entity *entity, double degree, double x, double y, double z);
void rotate_z(Entity *entity, double degree, double x, double y, double z);

void free_all_entities(Entity *entities);

#endif
