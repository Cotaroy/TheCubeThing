#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "space.h"

#define die(e) do { perror(e); exit(EXIT_FAILURE); } while (0);

Triangle *create_triangle(double *v1, double *v2, double *v3) {
  // make space for Triangle
  Triangle *new_triangle = malloc(sizeof(Triangle));
  if (new_triangle == NULL) {
    die("malloc");
  }
  new_triangle->vertex0 = v1;
  new_triangle->vertex1 = v2;
  new_triangle->vertex2 = v3;
  new_triangle->next = NULL;
  
  return new_triangle;
}

EntitySpace *create_space() {
    EntitySpace *space = malloc(sizeof(EntitySpace));
    for (int i = 0; i < MAX_ENTITIES; i++) {
        space->entity_list[0] = NULL;
    }
    for (int i = 0; i < MAX_LIGHTS; i++) {
        space->light_sources[0] = NULL;
    }
    return space;
}

Entity *create_entity(Triangle *object) {
  // make space for Entity
  Entity *new_entity = malloc(sizeof(Entity));
  if (new_entity == NULL) {
    die("malloc");
  }
  new_entity->object = object;

  return new_entity;
}

LightSource *create_light_source(double x, double y, double z, double intensity) {
    LightSource *source = malloc(sizeof(LightSource));
    source->x = x; source->y = y; source->z = z; source->intensity = intensity;
    return source;
}

void translate_light(LightSource *light_source, double x, double y, double z) {
    light_source->x += x;
    light_source->y += y;
    light_source->z += z;
}

void brighten(LightSource *light_source, double delta_intensity) {
    light_source->intensity += delta_intensity;
}

Entity *create_rectangle(double x, double y, double z, double x_length, double y_length, double z_length) {
  
  double *vertices[36];

  // allocate space for each vertex
  for (int i = 0; i < 36; i++) {
    vertices[i] = malloc(sizeof(double) * 3);
    // handle error
    if (vertices[i] == NULL) {
      for (int j = 0; j < i; j++) {
        free(vertices[j]);
      }
      die("malloc");
    }
  }

  // hard code each vertex ;-;
  vertices[0][0] = x; vertices[0][1] = y; vertices[0][2] = z;
  vertices[8][0] = x; vertices[8][1] = y; vertices[8][2] = z;
  vertices[9][0] = x; vertices[9][1] = y; vertices[9][2] = z;
  vertices[10][0] = x; vertices[10][1] = y; vertices[10][2] = z;

  vertices[1][0] = x; vertices[1][1] = y; vertices[1][2] = z + z_length;
  vertices[11][0] = x; vertices[11][1] = y; vertices[11][2] = z + z_length;
  vertices[12][0] = x; vertices[12][1] = y; vertices[12][2] = z + z_length;
  vertices[13][0] = x; vertices[13][1] = y; vertices[13][2] = z + z_length;
  vertices[14][0] = x; vertices[14][1] = y; vertices[14][2] = z + z_length;

  vertices[2][0] = x; vertices[2][1] = y + y_length; vertices[2][2] = z;
  vertices[15][0] = x; vertices[15][1] = y + y_length; vertices[15][2] = z;
  vertices[16][0] = x; vertices[16][1] = y + y_length; vertices[16][2] = z;
  vertices[17][0] = x; vertices[17][1] = y + y_length; vertices[17][2] = z;
  vertices[18][0] = x; vertices[18][1] = y + y_length; vertices[18][2] = z;

  vertices[3][0] = x; vertices[3][1] = y + y_length; vertices[3][2] = z + z_length;
  vertices[19][0] = x; vertices[19][1] = y + y_length; vertices[19][2] = z + z_length;
  vertices[20][0] = x; vertices[20][1] = y + y_length; vertices[20][2] = z + z_length;
  vertices[21][0] = x; vertices[21][1] = y + y_length; vertices[21][2] = z + z_length;

  vertices[4][0] = x + x_length; vertices[4][1] = y; vertices[4][2] = z;
  vertices[22][0] = x + x_length; vertices[22][1] = y; vertices[22][2] = z;
  vertices[23][0] = x + x_length; vertices[23][1] = y; vertices[23][2] = z;
  vertices[24][0] = x + x_length; vertices[24][1] = y; vertices[24][2] = z;
  vertices[25][0] = x + x_length; vertices[25][1] = y; vertices[25][2] = z;

  vertices[5][0] = x + x_length; vertices[5][1] = y; vertices[5][2] = z + z_length;
  vertices[26][0] = x + x_length; vertices[26][1] = y; vertices[26][2] = z + z_length;
  vertices[27][0] = x + x_length; vertices[27][1] = y; vertices[27][2] = z + z_length;
  vertices[28][0] = x + x_length; vertices[28][1] = y; vertices[28][2] = z + z_length;

  vertices[6][0] = x + x_length; vertices[6][1] = y + y_length; vertices[6][2] = z;
  vertices[29][0] = x + x_length; vertices[29][1] = y + y_length; vertices[29][2] = z;
  vertices[30][0] = x + x_length; vertices[30][1] = y + y_length; vertices[30][2] = z;
  vertices[31][0] = x + x_length; vertices[31][1] = y + y_length; vertices[31][2] = z;

  vertices[7][0] = x + x_length; vertices[7][1] = y + y_length; vertices[7][2] = z + z_length;
  vertices[32][0] = x + x_length; vertices[32][1] = y + y_length; vertices[32][2] = z + z_length;
  vertices[33][0] = x + x_length; vertices[33][1] = y + y_length; vertices[33][2] = z + z_length;
  vertices[34][0] = x + x_length; vertices[34][1] = y + y_length; vertices[34][2] = z + z_length;
  vertices[35][0] = x + x_length; vertices[35][1] = y + y_length; vertices[35][2] = z + z_length;

  // hard code each triangle FAHHHH
  Triangle *trig0 = create_triangle(vertices[0], vertices[1], vertices[5]);
  Triangle *trig1 = create_triangle(vertices[8], vertices[26], vertices[4]);
  trig0->next = trig1;

  Triangle *trig2 = create_triangle(vertices[22], vertices[27], vertices[7]);
  trig1->next = trig2;
  Triangle *trig3 = create_triangle(vertices[23], vertices[32], vertices[6]);
  trig2->next = trig3;

  Triangle *trig4 = create_triangle(vertices[29], vertices[33], vertices[3]);
  trig3->next = trig4;
  Triangle *trig5 = create_triangle(vertices[30], vertices[19], vertices[2]);
  trig4->next = trig5;
  
  Triangle *trig6 = create_triangle(vertices[15], vertices[20], vertices[11]);
  trig5->next = trig6;
  Triangle *trig7 = create_triangle(vertices[16], vertices[12], vertices[9]);
  trig6->next = trig7;

  Triangle *trig8 = create_triangle(vertices[13], vertices[21], vertices[34]);
  trig7->next = trig8;
  Triangle *trig9 = create_triangle(vertices[14], vertices[35], vertices[28]);
  trig8->next = trig9;

  Triangle *trig10 = create_triangle(vertices[17], vertices[10], vertices[24]);
  trig9->next = trig10;
  Triangle *trig11 = create_triangle(vertices[18], vertices[25], vertices[31]);
  trig10->next = trig11;

  Entity *rectangle = create_entity(trig0);
  rectangle->x_center = x + x_length/2;
  rectangle->y_center = y + y_length/2;
  rectangle->z_center = z + z_length/2;

  return rectangle;
}

void translate(Entity *entity, double x_offset, double y_offset, double z_offset) {
    Triangle *curr = entity->object;

    entity->x_center += x_offset;
    entity->y_center += y_offset;
    entity->z_center += z_offset;

    while(curr != NULL) {
        double *vertex0 = curr->vertex0;
        double *vertex1 = curr->vertex1;
        double *vertex2 = curr->vertex2;

        vertex0[0] += x_offset;
        vertex0[1] += y_offset;
        vertex0[2] += z_offset;

        vertex1[0] += x_offset;
        vertex1[1] += y_offset;
        vertex1[2] += z_offset;

        vertex2[0] += x_offset;
        vertex2[1] += y_offset;
        vertex2[2] += z_offset;

        curr = curr->next;
    }
}

void rotate_x_light(LightSource *entity, double degree, double x, double y, double z) {
    
    entity->x -= x;
    entity->y -= y;
    entity->z -= z;

    double cy = entity->y;
    double cz = entity->z;

    entity->y = cos(degree) * cy - sin(degree) * cz;
    entity->z = sin(degree) * cy + cos(degree) * cz;

    entity->x += x;
    entity->y += y;
    entity->z += z;

}

void rotate_y_light(LightSource *entity, double degree, double x, double y, double z) {

    entity->x -= x;
    entity->y -= y;
    entity->z -= z;

    double cx = entity->x;
    double cz = entity->z;

    entity->x = cos(degree) * cx + sin(degree) * cz;
    entity->z = - sin(degree) * cx + cos(degree) * cz;

    entity->x += x;
    entity->y += y;
    entity->z += z;

}

void rotate_z_light(LightSource *entity, double degree, double x, double y, double z) {

    entity->x -= x;
    entity->y -= y;
    entity->z -= z;

    double cx = entity->x;
    double cy = entity->y;

    entity->x = cos(degree) * cx - sin(degree) * cy;
    entity->y = sin(degree) * cx + cos(degree) * cy;

    entity->x += x;
    entity->y += y;
    entity->z += z;

}

void rotate_x(Entity *entity, double degree, double x, double y, double z) {
    Triangle *triangles = entity->object;
    Triangle *curr = triangles;

    entity->y_center -= y;
    entity->z_center -= z;

    double cy = entity->y_center;
    double cz = entity->z_center;

    entity->y_center = cos(degree) * cy - sin(degree) * cz;
    entity->z_center = sin(degree) * cy + cos(degree) * cz;

    entity->y_center += y;
    entity->z_center += z;

    while(curr != NULL) {
        double *vertex0 = curr->vertex0;
        double *vertex1 = curr->vertex1;
        double *vertex2 = curr->vertex2;

        vertex0[0] -= x;
        vertex0[1] -= y;
        vertex0[2] -= z;

        vertex1[0] -= x;
        vertex1[1] -= y;
        vertex1[2] -= z;

        vertex2[0] -= x;
        vertex2[1] -= y;
        vertex2[2] -= z;
        
        double v0y = vertex0[1];
        double v0z = vertex0[2];

        double v1y = vertex1[1];
        double v1z = vertex1[2];

        double v2y = vertex2[1];
        double v2z = vertex2[2];

        vertex0[1] = cos(degree) * v0y - sin(degree) * v0z;
        vertex0[2] = sin(degree) * v0y + cos(degree) * v0z;

        vertex1[1] = cos(degree) * v1y - sin(degree) * v1z;
        vertex1[2] = sin(degree) * v1y + cos(degree) * v1z;
        
        vertex2[1] = cos(degree) * v2y - sin(degree) * v2z;
        vertex2[2] = sin(degree) * v2y + cos(degree) * v2z;

        vertex0[0] += x;
        vertex0[1] += y;
        vertex0[2] += z;

        vertex1[0] += x;
        vertex1[1] += y;
        vertex1[2] += z;

        vertex2[0] += x;
        vertex2[1] += y;
        vertex2[2] += z;

        curr = curr->next;
    }

}

void rotate_y(Entity *entity, double degree, double x, double y, double z) {
    Triangle *triangles = entity->object;
    Triangle *curr = triangles;

    entity->x_center -= x;
    entity->z_center -= z;

    double cx = entity->x_center;
    double cz = entity->z_center;

    entity->x_center = cos(degree) * cx + sin(degree) * cz;
    entity->z_center = - sin(degree) * cx + cos(degree) * cz;

    entity->x_center += x;
    entity->z_center += z;

    while(curr != NULL) {
        double *vertex0 = curr->vertex0;
        double *vertex1 = curr->vertex1;
        double *vertex2 = curr->vertex2;

        vertex0[0] -= x;
        vertex0[1] -= y;
        vertex0[2] -= z;

        vertex1[0] -= x;
        vertex1[1] -= y;
        vertex1[2] -= z;

        vertex2[0] -= x;
        vertex2[1] -= y;
        vertex2[2] -= z;
        
        double v0x = vertex0[0];
        double v0z = vertex0[2];

        double v1x = vertex1[0];
        double v1z = vertex1[2];

        double v2x = vertex2[0];
        double v2z = vertex2[2];

        vertex0[0] = cos(degree) * v0x + sin(degree) * v0z;
        vertex0[2] = - sin(degree) * v0x + cos(degree) * v0z;

        vertex1[0] = cos(degree) * v1x + sin(degree) * v1z;
        vertex1[2] = - sin(degree) * v1x + cos(degree) * v1z;

        vertex2[0] = cos(degree) * v2x + sin(degree) * v2z;
        vertex2[2] = - sin(degree) * v2x + cos(degree) * v2z;

        vertex0[0] += x;
        vertex0[1] += y;
        vertex0[2] += z;

        vertex1[0] += x;
        vertex1[1] += y;
        vertex1[2] += z;

        vertex2[0] += x;
        vertex2[1] += y;
        vertex2[2] += z;

        curr = curr->next;
    }

}

void rotate_z(Entity *entity, double degree, double x, double y, double z) {
    Triangle *triangles = entity->object;
    Triangle *curr = triangles;

    entity->x_center -= x;
    entity->y_center -= y;

    double cx = entity->x_center;
    double cy = entity->y_center;

    entity->x_center = cos(degree) * cx - sin(degree) * cy;
    entity->y_center = sin(degree) * cx + cos(degree) * cy;

    entity->x_center += x;
    entity->y_center += y;

    while(curr != NULL) {
        double *vertex0 = curr->vertex0;
        double *vertex1 = curr->vertex1;
        double *vertex2 = curr->vertex2;

        vertex0[0] -= x;
        vertex0[1] -= y;
        vertex0[2] -= z;

        vertex1[0] -= x;
        vertex1[1] -= y;
        vertex1[2] -= z;

        vertex2[0] -= x;
        vertex2[1] -= y;
        vertex2[2] -= z;
        
        double v0x = vertex0[0];
        double v0y = vertex0[1];

        double v1x = vertex1[0];
        double v1y = vertex1[1];

        double v2x = vertex2[0];
        double v2y = vertex2[1];

        vertex0[0] = cos(degree) * v0x - sin(degree) * v0y;
        vertex0[1] = sin(degree) * v0x + cos(degree) * v0y;

        vertex1[0] = cos(degree) * v1x - sin(degree) * v1y;
        vertex1[1] = sin(degree) * v1x + cos(degree) * v1y;

        vertex2[0] = cos(degree) * v2x - sin(degree) * v2y;
        vertex2[1] = sin(degree) * v2x + cos(degree) * v2y;
        
        vertex0[0] += x;
        vertex0[1] += y;
        vertex0[2] += z;

        vertex1[0] += x;
        vertex1[1] += y;
        vertex1[2] += z;

        vertex2[0] += x;
        vertex2[1] += y;
        vertex2[2] += z;

        curr = curr->next;
    }

}

void free_all_triangles(Triangle *triangle) {
  if (triangle == NULL) {
    return;
  }
  free(triangle->vertex0);
  free(triangle->vertex1);
  free(triangle->vertex2);

  free_all_triangles(triangle->next);
  free(triangle);
}

void free_entity(Entity *entity) {
  
  if (entity == NULL) {
    return;
  }

  free_all_triangles(entity->object);

  free(entity);
  entity = NULL;
}

void free_space(EntitySpace *entities) {
    
    Entity **entity_list = entities->entity_list;

    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (entity_list[i] != NULL) {
            free_entity(entity_list[i]);
            entity_list[i] = NULL;
        }
    }
    for (int i = 0; i < MAX_LIGHTS; i++) {
        if (entities->light_sources[i] != NULL) {
            free(entities->light_sources[i]);
            entities->light_sources[i] = NULL;
        }
    }
    free(entities);
    entities = NULL;
}

void add_to_entity_space(EntitySpace *space, Entity *entity, int id) {
    delete_light_from_entity_space(space, id);
    space->entity_list[id] = entity;
}

void delete_from_entity_space(EntitySpace *space, int id) {
    free_entity(space->entity_list[id]);
    space->entity_list[id] = NULL;
}

void add_light_to_entity_space(EntitySpace *space, LightSource *entity, int id) {
    delete_light_from_entity_space(space, id);
    space->light_sources[id] = entity;
}

void delete_light_from_entity_space(EntitySpace *space, int id) {
    free(space->light_sources[id]);
    space->light_sources[id] = NULL;
}

Triangle *get_object(EntitySpace *space, int id) {
    if (space->entity_list[id] == NULL) {
        return NULL;
    }

    return space->entity_list[id]->object;

}

Entity *get_entity(EntitySpace *space, int id) {
    return space->entity_list[id];
}

LightSource *get_light(EntitySpace *space, int id) {
    return space->light_sources[id];
}
