#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "camera.h"
#include "space.h"
#include "raycast.h"
#include "renderer.h"

#define PI (3.14159265358979323846)

#define NUM_WORKERS (8)
#define HORIZONTAL_VIEW_ANGLE (PI / 4)
#define PIXEL_ASPECT_RATIO (11.0 / 21.0)

int main() {
    
    // build the scene
    EntitySpace *space = create_space();
    Entity *cube1 = create_rectangle(-.5, -.5, -.5, 1, 1, 1);
    Entity *cube2 = create_rectangle(-100, -100, 100, 200, 200, 1);
    add_to_entity_space(space, cube1, 1);
    add_to_entity_space(space, cube2, 2);

    // spawn the workers
    pid_t pids[NUM_WORKERS];
    int read_fds[NUM_WORKERS];
    int write_fds[NUM_WORKERS];
    spawn_camera_workers(pids, read_fds, write_fds, NUM_WORKERS, space);

    // create the film to capture the image on
    DistanceMap *map = malloc(sizeof(DistanceMap));
    map->width = 100;
    map->height = 10;
    map->distances = malloc(sizeof(double) * map->width * map->height);

    // render some stuff
    for (int i = 0; i < 60; i++) {
        translate(cube1, 0, 0, -10./65);
        rotate_z(cube1, PI/32, cube1->x_center, cube1->y_center, cube1->z_center);
        rotate_x(cube1, PI/32, cube1->x_center, cube1->y_center, cube1->z_center);
        rotate_y(cube1, PI/32, cube1->x_center, cube1->y_center, cube1->z_center);

        capture_image(
            map,
            HORIZONTAL_VIEW_ANGLE, // horizontal view angle
            PIXEL_ASPECT_RATIO,
            0, 0, -10, // camera position
            0, 0,      // camera rotation
            read_fds, write_fds, NUM_WORKERS);
        render(map);
    }

    // close the pipes and dispose of the children
    for (int i = 0; i < NUM_WORKERS; i++) {
        if (close(write_fds[i]) == -1) {
            perror("close");
            exit(1);
        }
    }
    for (int i = 0; i < NUM_WORKERS; i++) {
        int status;
        if (wait(&status) == -1) {
            perror("wait");
            exit(1);
        }
    }

    free(map->distances);
    free(map);
    free_space(space);

    return 0;
}


