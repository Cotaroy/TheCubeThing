#ifndef SPACE_H
#define SPACE_H
#define MAX_ENTITIES 10
#define MAX_LIGHTS 10

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

typedef struct light_source {
    double x;
    double y;
    double z;
    double intensity;
} LightSource;

// use the functions for adding to it and deleting from it
typedef struct entity_space {
    Entity *entity_list[MAX_ENTITIES];
    LightSource *light_sources[MAX_LIGHTS];
} EntitySpace;

// EntitySpace functions
// initializes lists to be NULL
// note that adding to the entity space already frees, whatever could have been at that index
EntitySpace *create_space();
void add_to_entity_space(EntitySpace *space, Entity *entity, int id);
void delete_from_entity_space(EntitySpace *space, int id);
void add_light_to_entity_space(EntitySpace *space, LightSource *entity, int id);
void delete_light_from_entity_space(EntitySpace *space, int id);
Triangle *get_object(EntitySpace *space, int id);
Entity *get_entity(EntitySpace *space, int id);
LightSource *get_light(EntitySpace *space, int id);
void free_space(EntitySpace *space);

// LightSource functions
LightSource *create_light_source(double x, double y, double z, double intensity);
void translate_light(LightSource *light_source, double x, double y, double z);
void brighten(LightSource *light_source, double delta_intensity);
void rotate_x_light(LightSource *entity, double degree, double x, double y, double z);
void rotate_y_light(LightSource *entity, double degree, double x, double y, double z);
void rotate_z_light(LightSource *entity, double degree, double x, double y, double z);

// (x,y,z) is the coordinate of the front left bottom vertex of the rectangle
Entity *create_rectangle(double x, double y, double z, double x_length, double y_length, double z_length);

// LINEAR TRANSFORMATION FUNCTIONS
// degree is the degree of rotation
// (x,y,z) is the position we are rotating around
void translate(Entity *entity, double x_offset, double y_offset, double z_offset);
void rotate_x(Entity *entity, double degree, double x, double y, double z);
void rotate_y(Entity *entity, double degree, double x, double y, double z);
void rotate_z(Entity *entity, double degree, double x, double y, double z);

void free_entity(Entity *entity);

#endif
