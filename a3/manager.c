#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include "camera.h"
#include "space.h"
#include "raycast.h"
#include "renderer.h"
#include "manager.h"
#include "controller.h"

#define PI (3.14159265358979323846)

#define NUM_WORKERS (8)
#define HORIZONTAL_VIEW_ANGLE (PI / 4)
#define PIXEL_ASPECT_RATIO (11.0 / 21.0)

/*
    The film size determines how much space is allocated to the
    distance map that the camera captures images onto.
    The camera may use less than the allocated space if the user's
    terminal window is smaller, but it cannot use more.
*/
#define FILM_MAX_WIDTH  (256)
#define FILM_MAX_HEIGHT (256)


void broadcast_to_pipes(int *write_fds, void *source_buffer, size_t nbytes) {
    for (int i = 0; i < NUM_WORKERS; i++) {

        // printf("writing to [%d]\n", i);
        
        if (write_safely(write_fds[i], source_buffer, nbytes) <= 0) {
            perror("write - broadcast");
            exit(1);
        }
    }
}

void broadcast_translate(EntitySpace *space, int *write_fds, int entity_id, double x_offset, double y_offset, double z_offset) {
    if (entity_id < 0 || entity_id >= MAX_ENTITIES) {
        fprintf(stderr, "Provided invalid entity_id (%d), out of bounds.\n", entity_id);
        return;
    }

    Entity *entity = get_entity(space, entity_id);

    if (entity == NULL) {
        fprintf(stderr, "Provided invalid entity_id (%d), perhaps it was deleted?\n", entity_id);
        return;
    }

    translate(entity, x_offset, y_offset, z_offset);

    CameraMessageHeader header = {0};
    CameraWorkerSpaceUpdate_TranslateEntity translation_details = {0};
    header.message_type = MSGTYPE_SPACE_UPDATE_TRANSLATE_ENTITY;
    translation_details.entity_id = entity_id;
    translation_details.x_offset = x_offset;
    translation_details.y_offset = y_offset;
    translation_details.z_offset = z_offset;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &translation_details, sizeof(CameraWorkerSpaceUpdate_TranslateEntity));
}

void broadcast_rotate(EntitySpace *space, int *write_fds, int entity_id, uint8_t axis_of_rotation, double angle, double x_center, double y_center, double z_center) {

    if (entity_id < 0 || entity_id >= MAX_ENTITIES) {
        fprintf(stderr, "Provided invalid entity_id (%d), out of bounds.\n", entity_id);
        return;
    }
    Entity *entity = get_entity(space, entity_id);
    if (entity == NULL) {
        fprintf(stderr, "Provided invalid entity_id (%d), perhaps it was deleted?\n", entity_id);
        return;
    }

    CameraMessageHeader header = {0};
    CameraWorkerSpaceUpdate_RotateEntity rotation_details = {0};
    header.message_type = MSGTYPE_SPACE_UPDATE_ROTATE_ENTITY;
    rotation_details.entity_id = entity_id;
    rotation_details.axis_of_rotation = axis_of_rotation;
    rotation_details.angle = angle;
    rotation_details.x_center = x_center;
    rotation_details.y_center = y_center;
    rotation_details.z_center = z_center;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &rotation_details, sizeof(CameraWorkerSpaceUpdate_RotateEntity));
    
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

void broadcast_translate_light(EntitySpace *space, int *write_fds, int entity_id, double x_offset, double y_offset, double z_offset) {
    if (entity_id < 0 || entity_id >= MAX_LIGHTS) {
        fprintf(stderr, "Provided invalid entity_id (%d), out of bounds.\n", entity_id);
        return;
    }
    LightSource *entity = get_light(space, entity_id);

    if (entity == NULL) {
        fprintf(stderr, "Provided invalid entity_id (%d), perhaps it was deleted?\n", entity_id);
        return;
    }

    CameraMessageHeader header = {0};
    CameraWorkerSpaceUpdate_TranslateLightSource translation_details = {0};
    header.message_type = MSGTYPE_SPACE_UPDATE_TRANSLATE_LIGHTSOURCE;
    translation_details.entity_id = entity_id;
    translation_details.x_offset = x_offset;
    translation_details.y_offset = y_offset;
    translation_details.z_offset = z_offset;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &translation_details, sizeof(CameraWorkerSpaceUpdate_TranslateLightSource));

    translate_light(entity, x_offset, y_offset, z_offset);
}

void broadcast_rotate_light(EntitySpace *space, int *write_fds, int entity_id, uint8_t axis_of_rotation, double angle, double x_center, double y_center, double z_center) {
    if (entity_id < 0 || entity_id >= MAX_LIGHTS) {
        fprintf(stderr, "Provided invalid entity_id (%d), out of bounds.\n", entity_id);
        return;
    }
    LightSource *entity = get_light(space, entity_id);

    if (entity == NULL) {
        fprintf(stderr, "Provided invalid entity_id (%d), perhaps it was deleted?\n", entity_id);
        return;
    }

    CameraMessageHeader header = {0};
    CameraWorkerSpaceUpdate_RotateLightSource rotation_details = {0};
    header.message_type = MSGTYPE_SPACE_UPDATE_ROTATE_LIGHTSOURCE;
    rotation_details.entity_id = entity_id;
    rotation_details.axis_of_rotation = axis_of_rotation;
    rotation_details.angle = angle;
    rotation_details.x_center = x_center;
    rotation_details.y_center = y_center;
    rotation_details.z_center = z_center;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &rotation_details, sizeof(CameraWorkerSpaceUpdate_RotateLightSource));
    
    if (axis_of_rotation == MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_X) {
        rotate_x_light(entity, angle, x_center, y_center, z_center);
    }
    else if (axis_of_rotation == MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_Y) {
        rotate_y_light(entity, angle, x_center, y_center, z_center);
    }
    else if (axis_of_rotation == MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_Z) {
        rotate_z_light(entity, angle, x_center, y_center, z_center);
    }
}

void broadcast_brighten_light(EntitySpace *space, int *write_fds, int entity_id, double delta_intensity) {
    if (entity_id < 0 || entity_id >= MAX_LIGHTS) {
        fprintf(stderr, "Provided invalid entity_id (%d), out of bounds.\n", entity_id);
        return;
    }
    LightSource *source = get_light(space, entity_id);

    if (source == NULL) {
        fprintf(stderr, "Provided invalid entity_id (%d), perhaps it was deleted?\n", entity_id);
        return;
    }

    CameraMessageHeader header = {0};
    CameraWorkerSpaceUpdate_BrightenLightSource brighten_details = {0};
    header.message_type = MSGTYPE_SPACE_UPDATE_BRIGHTEN_LIGHTSOURCE;
    brighten_details.entity_id = entity_id;
    brighten_details.delta_intensity = delta_intensity;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &brighten_details, sizeof(CameraWorkerSpaceUpdate_BrightenLightSource));

    brighten(source, delta_intensity);
}

Entity *broadcast_create_entity(EntitySpace *space, int *write_fds, int entity_id, double x_corner, double y_corner, double z_corner, double x_length, double y_length, double z_length) {
    if (entity_id < 0 || entity_id >= MAX_ENTITIES) {
        fprintf(stderr, "Provided invalid entity_id (%d), out of bounds.\n", entity_id);
        return NULL;
    }
    CameraMessageHeader header = {0};
    CameraWorkerSpaceUpdate_NewEntity new_details = {0};
    header.message_type = MSGTYPE_SPACE_UPDATE_NEW_ENTITY;
    new_details.entity_id = entity_id;
    new_details.corner_coord[0] = x_corner; new_details.corner_coord[1] = y_corner; new_details.corner_coord[2] = z_corner;
    new_details.side_lengths[0] = x_length; new_details.side_lengths[1] = y_length; new_details.side_lengths[2] = z_length;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &new_details, sizeof(CameraWorkerSpaceUpdate_NewEntity));

    Entity *entity = create_rectangle(x_corner, y_corner, z_corner, x_length, y_length, z_length);
    add_to_entity_space(space, entity, entity_id);

    return entity;
}

LightSource *broadcast_create_light_source(EntitySpace *space, int *write_fds, int entity_id, double x, double y, double z, double intensity) {
    if (entity_id < 0 || entity_id >= MAX_LIGHTS) {
        fprintf(stderr, "Provided invalid entity_id (%d), out of bounds.\n", entity_id);
        return NULL;
    }
    CameraMessageHeader header = {0};
    CameraWorkerSpaceUpdate_NewLightSource new_details = {0};
    header.message_type = MSGTYPE_SPACE_UPDATE_NEW_LIGHTSOURCE;
    new_details.entity_id = entity_id;
    new_details.coord[0] = x; new_details.coord[1] = y; new_details.coord[2] = z;
    new_details.intensity = intensity;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &new_details, sizeof(CameraWorkerSpaceUpdate_NewLightSource));

    LightSource *source = create_light_source(x, y, z, intensity);
    add_light_to_entity_space(space, source, entity_id);

    return source;
}

void broadcast_delete_entity(EntitySpace *space, int *write_fds, int entity_id) {
    if (entity_id < 0 || entity_id >= MAX_ENTITIES) {
        fprintf(stderr, "Provided invalid entity_id (%d), out of bounds.\n", entity_id);
        return;
    }
    CameraMessageHeader header = {0};
    CameraWorkerSpaceUpdate_DeleteEntity delete_details = {0};
    header.message_type = MSGTYPE_SPACE_UPDATE_DELETE_ENTITY;
    delete_details.entity_id = entity_id;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &delete_details, sizeof(CameraWorkerSpaceUpdate_DeleteEntity));

    delete_from_entity_space(space, entity_id);
}

void broadcast_delete_light_source(EntitySpace *space, int *write_fds, int entity_id) {
    if (entity_id < 0 || entity_id >= MAX_ENTITIES) {
        fprintf(stderr, "Provided invalid entity_id (%d), out of bounds.\n", entity_id);
        return;
    }
    CameraMessageHeader header = {0};
    CameraWorkerSpaceUpdate_DeleteLightSource delete_details = {0};
    header.message_type = MSGTYPE_SPACE_UPDATE_DELETE_LIGHTSOURCE;
    delete_details.entity_id = entity_id;
    broadcast_to_pipes(write_fds, &header, sizeof(CameraMessageHeader));
    broadcast_to_pipes(write_fds, &delete_details, sizeof(CameraWorkerSpaceUpdate_DeleteLightSource));

    delete_light_from_entity_space(space, entity_id);
}

int terminal_width = 32;
int terminal_height = 32;

void cleanup_and_exit(int sig) { 
    terminal_exit_alt_screen();
    restore_original_settings();
    exit(sig); // i don't know what exit code to use here
}
void update_window_size(int) {
    // https://www.man7.org/linux/man-pages/man2/TIOCSWINSZ.2const.html
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) < 0) {
        perror("Failed to update window size.");
        // not exiting because it seems like
        // this shouldn't be a fatal error
        return;
    }
    terminal_width = fmin(FILM_MAX_WIDTH, w.ws_col);
    terminal_height = fmin(FILM_MAX_HEIGHT, w.ws_row);
}

int main() {
    // set up signal handlers
    // https://en.wikipedia.org/wiki/Signal_(IPC)
    if (signal(SIGTERM, cleanup_and_exit) == SIG_ERR ||
        signal(SIGINT, cleanup_and_exit) == SIG_ERR ||
        signal(SIGHUP, cleanup_and_exit) == SIG_ERR ||
        signal(SIGTSTP, cleanup_and_exit) == SIG_ERR ||
        signal(SIGWINCH, update_window_size) == SIG_ERR
    ) {
        perror("signal");
        exit(1);
    }

    // update window size once on startup to give it the correct values
    update_window_size(SIGWINCH);

    // setup handling camera movement through user input
    setup_non_canonical();
    double camera_x = -10;
    double camera_y = 0;
    double camera_z = 0;
    double camera_azimuth = 0;
    double camera_inclination = PI/2;

    // build the scene
    EntitySpace *space = create_space();

    // spawn the workers
    pid_t pids[NUM_WORKERS];
    int read_fds[NUM_WORKERS];
    int write_fds[NUM_WORKERS];
    spawn_camera_workers(pids, read_fds, write_fds, NUM_WORKERS, space);

    // create the film to capture the image on
    DistanceMap *map = malloc(sizeof(DistanceMap));
    map->width = terminal_width;
    map->height = terminal_height;
    map->distances = malloc(sizeof(double) * FILM_MAX_WIDTH * FILM_MAX_HEIGHT);

    terminal_enter_alt_screen();

    Entity *cube1 = broadcast_create_entity(space, write_fds, 0, -.5, -.5, -.5, 1, 1, 1);
    broadcast_create_light_source(space, write_fds, 0, 0, 0, 2, 1000);
    broadcast_create_light_source(space, write_fds, 1, 0, 0, -2, 1000);
    broadcast_create_light_source(space, write_fds, 2, 2, 0, 0, 1000);
    broadcast_create_light_source(space, write_fds, 3, -2, 0, 0, 1000);
    broadcast_create_light_source(space, write_fds, 4, 0, 2, 0, 1000);
    broadcast_create_light_source(space, write_fds, 5, 0, -2, 0, 1000);

    // render some stuff
    for (int i = 0; i < 100; i++) {
        /*
            Assumption:
            Between frames, the pipes used to write to the children
            should be empty.
            I don't know if this is always actually true.
        */

        // make sure the film is always in sync with the user's window size
        map->width = terminal_width;
        map->height = terminal_height;

        handle_non_canonical_input(&camera_x, &camera_y, &camera_z, &camera_azimuth, &camera_inclination);
        printf("(%f, %f, %f, %f, %f)\n", camera_x, camera_y, camera_z, camera_azimuth, camera_inclination);


        // broadcast_translate(space, write_fds, 2, 0, -3./60, 0);

        // broadcast_brighten_light(space, write_fds, 0, -(source->intensity * .1));

        // broadcast_rotate(space, write_fds, 2, MSGDETAIL_ROTATE_ENTITY_AXIS_X, PI/32, cube2->x_center, cube2->y_center, cube2->z_center);
        // broadcast_rotate(space, write_fds, 2, MSGDETAIL_ROTATE_ENTITY_AXIS_Y, PI/32, cube2->x_center, cube2->y_center, cube2->z_center);
        // broadcast_rotate(space, write_fds, 2, MSGDETAIL_ROTATE_ENTITY_AXIS_Z, PI/32, cube2->x_center, cube2->y_center, cube2->z_center);
        
        // broadcast_rotate_light(space, write_fds, 0, MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_X, PI/32, 0, 0, 0);
        // broadcast_rotate_light(space, write_fds, 0, MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_Y, PI/32, cube1->x_center, cube1->y_center, cube1->z_center);
        // broadcast_rotate_light(space, write_fds, 0, MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_Z, PI/32, 0, 0, 0);
    
        broadcast_rotate(space, write_fds, 0, MSGDETAIL_ROTATE_ENTITY_AXIS_X, -PI/64, cube1->x_center, cube1->y_center, cube1->z_center);
        broadcast_rotate(space, write_fds, 0, MSGDETAIL_ROTATE_ENTITY_AXIS_Y, -PI/64, cube1->x_center, cube1->y_center, cube1->z_center);
        broadcast_rotate(space, write_fds, 0, MSGDETAIL_ROTATE_ENTITY_AXIS_Z, -PI/64, cube1->x_center, cube1->y_center, cube1->z_center);

        capture_image(
            map,
            HORIZONTAL_VIEW_ANGLE, // horizontal view angle
            PIXEL_ASPECT_RATIO,
            camera_x, camera_y, camera_z, // camera position
            camera_azimuth, camera_inclination,      // camera rotation
            read_fds, write_fds, NUM_WORKERS);
        terminal_move_cursor_to_topleft();
        render_luminosity(map);
    }

    terminal_exit_alt_screen();

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

    restore_original_settings(&og_settings);
    free(map->distances);
    free(map);
    free_space(space);

    return 0;
}
