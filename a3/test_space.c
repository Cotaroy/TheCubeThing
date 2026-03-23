#include <stdio.h>
#include "space.h"

int main() {

  Vertex *vertices = NULL;
  Entity *unit_cube = create_rectangle(NULL, &vertices, -1, -1, 8, 3, 3, 3);
  
  Entity *curr = unit_cube;
  while (curr != NULL) {
    Triangle *obj = curr->object;
    Triangle *curr1 = obj;

    while (curr1 != NULL) {
      printf("[");
      printf("(%.0f, %.0f, %.0f), ", curr1->vertex0[0], curr1->vertex0[1], curr1->vertex0[2]);
      printf("(%.0f, %.0f, %.0f), ", curr1->vertex1[0], curr1->vertex1[1], curr1->vertex1[2]);
      printf("(%.0f, %.0f, %.0f)]\n", curr1->vertex2[0], curr1->vertex2[1], curr1->vertex2[2]);

      curr1 = curr1->next;
    }

    curr = curr->next;
  }

  free_all_entities(unit_cube);
  free_all_vertices(vertices);

  return 0;
}
