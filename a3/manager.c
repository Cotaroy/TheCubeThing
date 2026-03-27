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


static void broadcast_to_pipes(int *write_fds, void *source_buffer, size_t nbytes) {
    for (int i = 0; i < NUM_WORKERS; i++) {

        // printf("writing to [%d]\n", i);
        
        if (write_safely(write_fds[i], source_buffer, nbytes) <= 0) {
            perror("write - broadcast");
            exit(1);
        }
    }
}


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
    map->width = 120;
    map->height = 40;
    map->distances = malloc(sizeof(double) * map->width * map->height);

    // repeatedly overwritten
    CameraMessageHeader *header = malloc(sizeof(CameraMessageHeader));
    CameraWorkerSpaceUpdate_TranslateEntity *translation_details = malloc(sizeof(CameraWorkerSpaceUpdate_TranslateEntity));
    CameraWorkerSpaceUpdate_RotateEntity *rotation_details = malloc(sizeof(CameraWorkerSpaceUpdate_RotateEntity));

    // render some stuff
    for (int i = 0; i < 60; i++) {
        /*
            Assumption:
            Between frames, the pipes used to write to the children
            should be empty.
            I don't know if this is always actually true.
        */

        header->message_type = MSGTYPE_SPACE_UPDATE_TRANSLATE_ENTITY;
        translation_details->entity_id = 1; // hardcoded temporarily
        translation_details->x_offset = 0;
        translation_details->y_offset = 0;
        translation_details->z_offset = -10/65.0;
        broadcast_to_pipes(write_fds, header, sizeof(*header));
        broadcast_to_pipes(write_fds, translation_details, sizeof(*translation_details));
        translate(cube1, 0, 0, -10./65);

        
        header->message_type = MSGTYPE_SPACE_UPDATE_ROTATE_ENTITY;
        
        
        rotation_details->axis_of_rotation = MSGDETAIL_ROTATE_ENTITY_AXIS_Z;
        rotation_details->entity_id = 1;
        rotation_details->angle = PI / 32;
        rotation_details->x_center = cube1->x_center;
        rotation_details->y_center = cube1->y_center;
        rotation_details->z_center = cube1->z_center;

        broadcast_to_pipes(write_fds, header, sizeof(*header));
        broadcast_to_pipes(write_fds, rotation_details,
            sizeof(*rotation_details));
        rotate_z(cube1, PI / 32, cube1->x_center, cube1->y_center,
                    cube1->z_center);

        rotation_details->axis_of_rotation = MSGDETAIL_ROTATE_ENTITY_AXIS_X;
        rotation_details->entity_id = 1;
        rotation_details->angle = PI / 32;
        rotation_details->x_center = cube1->x_center;
        rotation_details->y_center = cube1->y_center;
        rotation_details->z_center = cube1->z_center;

        broadcast_to_pipes(write_fds, header, sizeof(*header));
        broadcast_to_pipes(write_fds, rotation_details,
            sizeof(*rotation_details));
            rotate_x(cube1, PI / 32, cube1->x_center, cube1->y_center,
                     cube1->z_center);

        rotation_details->axis_of_rotation = MSGDETAIL_ROTATE_ENTITY_AXIS_Y;
        rotation_details->entity_id = 1;
        rotation_details->angle = PI / 32;
        rotation_details->x_center = cube1->x_center;
        rotation_details->y_center = cube1->y_center;
        rotation_details->z_center = cube1->z_center;
        
        broadcast_to_pipes(write_fds, header, sizeof(*header));
        broadcast_to_pipes(write_fds, rotation_details,
            sizeof(*rotation_details));
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