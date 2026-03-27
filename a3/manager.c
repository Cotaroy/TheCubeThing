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

static void broadcast_translate(EntitySpace *space, int *write_fds, int entity_id, double x_offset, double y_offset, double z_offset) {
    CameraMessageHeader header;
    CameraWorkerSpaceUpdate_TranslateEntity translation_details;
    header.message_type = MSGTYPE_SPACE_UPDATE_TRANSLATE_ENTITY;
    translation_details.entity_id = entity_id;
    translation_details.x_offset = x_offset;
    translation_details.y_offset = y_offset;
    translation_details.z_offset = z_offset;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &translation_details, sizeof(CameraWorkerSpaceUpdate_TranslateEntity));
    
    Entity *entity = get_entity(space, entity_id);
    translate(entity, x_offset, y_offset, z_offset);
}

static void broadcast_rotate(EntitySpace *space, int *write_fds, int entity_id, uint8_t axis_of_rotation, double angle, double x_center, double y_center, double z_center) {
    CameraMessageHeader header;
    CameraWorkerSpaceUpdate_RotateEntity rotation_details;
    header.message_type = MSGTYPE_SPACE_UPDATE_ROTATE_ENTITY;
    rotation_details.entity_id = entity_id;
    rotation_details.axis_of_rotation = axis_of_rotation;
    rotation_details.angle = angle;
    rotation_details.x_center = x_center;
    rotation_details.y_center = y_center;
    rotation_details.z_center = z_center;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &rotation_details, sizeof(CameraWorkerSpaceUpdate_RotateEntity));
    
    Entity *entity = get_entity(space, entity_id);
    if (axis_of_rotation == MSGDETAIL_ROTATE_ENTITY_AXIS_X) {
        rotate_x(entity, angle, x_center, y_center, z_center);
    }
    else if (axis_of_rotation == MSGDETAIL_ROTATE_ENTITY_AXIS_Y) {
        rotate_y(entity, angle, x_center, y_center, z_center);
    }
    else if (axis_of_rotation == MSGDETAIL_ROTATE_ENTITY_AXIS_Z) {
        rotate_z(entity, angle, x_center, y_center, z_center);
    }
}

int main() {
    
    // build the scene
    EntitySpace *space = create_space();
    Entity *cube1 = create_rectangle(-.5, -.5, -.5, 1, 1, 1);
    Entity *cube2 = create_rectangle(-100, -100, 100, 200, 200, 1);
    LightSource *source = create_light_source(-3, 0, -5, 1000);
    // LightSource *source2 = create_light_source(-3, -3, -3, 1000);
    add_light_to_entity_space(space, source, 0);
    // add_light_to_entity_space(space, source2, 1);
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

    // render some stuff
    for (int i = 0; i < 60; i++) {
        /*
            Assumption:
            Between frames, the pipes used to write to the children
            should be empty.
            I don't know if this is always actually true.
        */

        // broadcast_translate(space,write_fds, 1, 0, 0, -10/65.0);
        broadcast_rotate(space, write_fds, 1, MSGDETAIL_ROTATE_ENTITY_AXIS_X, PI/32, cube1->x_center, cube1->y_center, cube1->z_center);
        broadcast_rotate(space, write_fds, 1, MSGDETAIL_ROTATE_ENTITY_AXIS_Y, PI/32, cube1->x_center, cube1->y_center, cube1->z_center);
        broadcast_rotate(space, write_fds, 1, MSGDETAIL_ROTATE_ENTITY_AXIS_Z, PI/32, cube1->x_center, cube1->y_center, cube1->z_center);

        capture_image(
            map,
            HORIZONTAL_VIEW_ANGLE, // horizontal view angle
            PIXEL_ASPECT_RATIO,
            0, 0, -10, // camera position
            0, 0,      // camera rotation
            read_fds, write_fds, NUM_WORKERS);
        render_luminosity(map);
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
