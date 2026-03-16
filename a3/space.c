#include <stdio.h>
#include <stdlib.h>
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

Triangle *create_triangle_v(Vertex *v1, Vertex *v2, Vertex *v3) {

  // make space for Triangle
  Triangle *new_triangle = malloc(sizeof(Triangle));
  if (new_triangle == NULL) {
    die("malloc");
  }
  new_triangle->vertex0 = v1->coordinate;
  new_triangle->vertex1 = v2->coordinate;
  new_triangle->vertex2 = v3->coordinate;
  new_triangle->next = NULL;

  return new_triangle;
}

Entity *create_entity(Triangle *object) {
  // make space for Entity
  Entity *new_entity = malloc(sizeof(Entity));
  if (new_entity == NULL) {
    die("malloc");
  }
  new_entity->object = object;
  new_entity->next = NULL;

  return new_entity;
}

Vertex *create_vertex(double *coordinate) {

  Vertex *new_vertex = malloc(sizeof(Vertex));
  if (new_vertex == NULL) {
    die("malloc");
  }
  new_vertex->coordinate = coordinate;
  new_vertex->next = NULL;

  return new_vertex;
}

// the centre will always be the origin
Entity *create_rectangle(Entity *entities, Vertex **vertex_list, double x, double y, double z, double x_length, double y_length, double z_length) {
  
  double *vertices[8];

  // allocate space for each vertex
  for (int i = 0; i < 8; i++) {
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
  vertices[1][0] = x; vertices[1][1] = y; vertices[1][2] = z + z_length;
  vertices[2][0] = x; vertices[2][1] = y + y_length; vertices[2][2] = z;
  vertices[3][0] = x; vertices[3][1] = y + y_length; vertices[3][2] = z + z_length;
  vertices[4][0] = x + x_length; vertices[4][1] = y; vertices[4][2] = z;
  vertices[5][0] = x + x_length; vertices[5][1] = y; vertices[5][2] = z + z_length;
  vertices[6][0] = x + x_length; vertices[6][1] = y + y_length; vertices[6][2] = z;
  vertices[7][0] = x + x_length; vertices[7][1] = y + y_length; vertices[7][2] = z + z_length;

  // hard code each triangle FAHHHH
  Triangle *trig0 = create_triangle(vertices[0], vertices[1], vertices[5]);
  Triangle *trig1 = create_triangle(vertices[0], vertices[4], vertices[5]);
  trig0->next = trig1;

  Triangle *trig2 = create_triangle(vertices[4], vertices[5], vertices[7]);
  trig1->next = trig2;
  Triangle *trig3 = create_triangle(vertices[4], vertices[6], vertices[7]);
  trig2->next = trig3;

  Triangle *trig4 = create_triangle(vertices[2], vertices[3], vertices[7]);
  trig3->next = trig4;
  Triangle *trig5 = create_triangle(vertices[2], vertices[6], vertices[7]);
  trig4->next = trig5;
  
  Triangle *trig6 = create_triangle(vertices[0], vertices[1], vertices[3]);
  trig5->next = trig6;
  Triangle *trig7 = create_triangle(vertices[0], vertices[2], vertices[3]);
  trig6->next = trig7;

  Triangle *trig8 = create_triangle(vertices[1], vertices[3], vertices[7]);
  trig7->next = trig8;
  Triangle *trig9 = create_triangle(vertices[1], vertices[5], vertices[7]);
  trig8->next = trig9;

  Triangle *trig10 = create_triangle(vertices[0], vertices[2], vertices[6]);
  trig9->next = trig10;
  Triangle *trig11 = create_triangle(vertices[0], vertices[4], vertices[6]);
  trig10->next = trig11;

  // add all vertices to vertex list
  for (int i = 0; i < 8; i++) {
    Vertex *new_vertex = create_vertex(vertices[i]);
    new_vertex->next = *vertex_list;
    *vertex_list = new_vertex;
  }

  Entity *rectangle = create_entity(trig0);
  rectangle->next = entities;
  return rectangle;
}

void free_all_triangles(Triangle *triangle) {
  if (triangle == NULL) {
    return;
  }

  free_all_triangles(triangle->next);
  free(triangle);
}

void free_all_entities(Entity *entities) {
  
  if (entities == NULL) {
    return;
  }

  free_all_triangles(entities->object);

  free_all_entities(entities->next);
  free(entities);

} 

void free_all_vertices(Vertex *vertices) {
  if (vertices == NULL) 
  {
    return;
  }

  free(vertices->coordinate);

  free_all_vertices(vertices->next);
  free(vertices);
}

int main() {

  Vertex *vertices = NULL;
  Entity *unit_cube = create_rectangle(NULL, &vertices, 0, 0, 0, 1, 1, 1);
  
  Entity *curr = unit_cube;
  while (curr != NULL) {
    Triangle *obj = curr->object;
    Triangle *curr1 = obj;

    while (curr1 != NULL) {
      printf("[");
      printf("(%f, %f, %f), ", curr1->vertex0[0], curr1->vertex0[1], curr1->vertex0[2]);
      printf("(%f, %f, %f), ", curr1->vertex1[0], curr1->vertex1[1], curr1->vertex1[2]);
      printf("(%f, %f, %f)]\n", curr1->vertex2[0], curr1->vertex2[1], curr1->vertex2[2]);

      curr1 = curr1->next;
    }

    curr = curr->next;
  }

  free_all_entities(unit_cube);
  free_all_vertices(vertices);

  return 0;
}
