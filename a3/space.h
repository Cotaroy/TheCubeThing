#ifndef SPACE_H
#define SPACE_H
#define MAX_ENTITIES 10

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
} Entity;

// use the functions for adding to it and deleting from it
typedef struct entity_space {
    Entity *entity_list[MAX_ENTITIES];
} EntitySpace;

EntitySpace *create_space();
void add_to_entity_space(EntitySpace *space, Entity *entity, int id);
void remove_from_entity_space(EntitySpace *space, Entity *entity, int id);
Triangle *get_object(EntitySpace *space, int id);

// this struct is mainly for freeing vertices (fix double freeing problem if same vertex used twice)
typedef struct vertex {
  double *coordinate;
  struct vertex *next; // vertices will be NULL terminated
} Vertex; 

// (x,y,z) is the coordinate of the front left bottom vertex of the rectangle
Entity *create_rectangle(double x, double y, double z, double x_length, double y_length, double z_length);

// degree is the degree of rotation
// (x,y,z) is the position we are rotating around
void translate(Entity *entity, double x_offset, double y_offset, double z_offset);
void rotate_x(Entity *entity, double degree, double x, double y, double z);
void rotate_y(Entity *entity, double degree, double x, double y, double z);
void rotate_z(Entity *entity, double degree, double x, double y, double z);

void free_entity(Entity *entity);
void free_space(EntitySpace *entities);

#endif
