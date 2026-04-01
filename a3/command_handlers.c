#include "camera.h"
#include "controller.h"
#include "manager.h"
#include "parser.h"
#include "space.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PI (3.14159265358979323846)

int __handle_command__exit(int argc, char **argv) {
    // suppress unused variable warnings
    // because actually just don't need these
    (void)argc;
    (void)argv;

    printf("byebye\n");
    EntitySpace *space = get_space();
    restore_original_settings();

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

    free_space(space);
    exit(0);
}

int __handle_command__list(int argc, char **argv) {
    // suppress unused variable warnings
    // because actually just don't need these
    (void)argc;
    (void)argv;

    printf("Entities & Light Sources in the space:\n");
    printf("======================================\n");

    EntitySpace *space = get_space();
    for (int entity_id = 0; entity_id < MAX_ENTITIES; entity_id++) {
        Entity *entity = space->entity_list[entity_id];
        if (entity == NULL) {
            continue;
        }
        printf("[entity %d] at (%.1f, %.1f, %.1f)\n",
               entity_id,
               entity->x_center,
               entity->y_center,
               entity->z_center);
    }
    for (int light_id = 0; light_id < MAX_ENTITIES; light_id++) {
        LightSource *light = space->light_sources[light_id];
        if (light == NULL) {
            continue;
        }
        printf("[light  %d] at (%.1f, %.1f, %.1f, %.1f)\n",
               light_id,
               light->x,
               light->y,
               light->z,
               light->intensity);
    }

    fflush(stdout); // force the printed stuff to go to the terminal immediately
    return 0;
}

int __handle_command__delete(int argc, char **argv) {
    if (argc < 3) {
        printf("Syntax: %s <e[ntity]|l[ight]> <id>\n", argv[0]);
        fflush(stdout);
        return 1;
    }

    errno = 0;
    int id = (int)strtol(argv[2], NULL, 10);
    if (errno != 0) {
        printf("You must specify a numeric ID.\n");
        fflush(stdout);
        return 1;
    }

    EntitySpace *space = get_space();
    if (argv[1][0] == 'e') {
        if (id < 0 || id >= MAX_ENTITIES) {
            printf("invalid id\n");
            fflush(stdout);
            return 1;
        }
        broadcast_delete_entity(space, id);
        printf("deleted entity %d\n", id);
    } else if (argv[1][0] == 'l') {
        if (id < 0 || id >= MAX_LIGHTS) {
            printf("invalid id\n");
            fflush(stdout);
            return 1;
        }
        broadcast_delete_light_source(space, id);
        printf("deleted light %d\n", id);
    } else {
        printf("You must specify 'e' or 'l' for entity or light.\n");
        fflush(stdout);
        return 1;
    }

    fflush(stdout);
    return 0;
}

int __handle_command__translate(int argc, char **argv) {
    if (argc < 6) {
        printf("Syntax: %s <e[ntity]|l[ight]> <id> <x offset> <y offset> <z "
               "offset>",
               argv[0]);
        fflush(stdout);
        return 1;
    }

    errno = 0;
    int id = (int)strtol(argv[2], NULL, 10);
    if (errno != 0) {
        printf("You must specify a numeric ID.\n");
        fflush(stdout);
        return 1;
    }

    errno = 0;
    double x_offset = strtod(argv[3], NULL);
    double y_offset = strtod(argv[4], NULL);
    double z_offset = strtod(argv[5], NULL);
    if (errno != 0) {
        printf("Invalid x, y, or z offset.\n");
        fflush(stdout);
        return 1;
    }

    EntitySpace *space = get_space();
    if (argv[1][0] == 'e') {
        if (id < 0 || id >= MAX_ENTITIES) {
            printf("invalid id\b");
            fflush(stdout);
            return 1;
        }
        Entity *entity = get_entity(space, id);
        if (entity == NULL) {
            printf("unknown entity\n");
            fflush(stdout);
            return 1;
        }
        broadcast_translate(space, id, x_offset, y_offset, z_offset);
        printf("translated entity\n");
    } else if (argv[1][0] == 'l') {
        if (id < 0 || id >= MAX_LIGHTS) {
            printf("invalid id\n");
            fflush(stdout);
            return 1;
        }
        LightSource *light = get_light(space, id);
        if (light == NULL) {
            printf("unknown light\n");
            fflush(stdout);
            return 1;
        }
        broadcast_translate_light(space, id, x_offset, y_offset, z_offset);
        printf("translated light\n");
    } else {
        printf("You must specify 'e' or 'l' for entity or light.\n");
        fflush(stdout);
        return 1;
    }

    fflush(stdout);
    return 0;
}

int __handle_command__brighten(int argc, char **argv) {
    if (argc < 3) {
        printf("Syntax: %s <id> <delta_intensity>\n", argv[0]);
        fflush(stdout);
        return 1;
    }

    errno = 0;
    int id = (int)strtol(argv[1], NULL, 10);
    if (errno != 0) {
        printf("You must specify a numeric ID.\n");
        fflush(stdout);
        return 1;
    }

    errno = 0;
    double delta_intensity = strtod(argv[2], NULL);
    if (errno != 0) {
        printf("You must specify a numeric delta_intensity.\n");
        fflush(stdout);
        return 1;
    }
    
    EntitySpace *space = get_space();
    broadcast_brighten_light(space, id, delta_intensity);
    return 0;
}

int __handle_command__rotate(int argc, char **argv) {
    if (argc < 8) {
        printf("Syntax: %s <e[ntity]|l[ight]> <id> <x|y|z (axis of rotation)> "
               "<angle in degrees counterclockwise> <c[enter]|x_center> <c[enter]|y_center> <c[enter]|z_center>\n",
               argv[0]);
        fflush(stdout);
        return 1;
    }

    errno = 0;
    int id = (int)strtol(argv[2], NULL, 10);
    if (errno != 0) {
        printf("You must specify a numeric ID.\n");
        fflush(stdout);
        return 1;
    }
    if (argv[1][0] == 'e' && (id < 0 || id >= MAX_ENTITIES)) {
        printf("invalid id\n");
        fflush(stdout);
        return 1;
    }
    if (argv[1][0] == 'l' && (id < 0 || id >= MAX_LIGHTS)) {
        printf("invalid id\n");
        fflush(stdout);
        return 1;
    }

    if (argv[3][0] != 'x' && argv[3][0] != 'y' && argv[3][0] != 'z') {
        printf("specify x, y, or z as axis of rotation.\n");
        fflush(stdout);
        return 1;
    }

    errno = 0;
    double angle_in_degrees = strtod(argv[4], NULL);
    if (errno != 0) {
        printf("Invalid angle.\n");
        fflush(stdout);
        return 1;
    }
    double angle_in_radians = PI * angle_in_degrees / 180;


    EntitySpace *space = get_space();
    if (argv[1][0] == 'e') {
        Entity *entity = get_entity(space, id);
        if (entity == NULL) {
            printf("unknown entity\n");
            fflush(stdout);
            return 1;
        }

        errno = 0;
        double x_center = strtod(argv[5], NULL);
        if (errno != 0 && argv[5][0] != 'c') {
            printf("Invalid center of rotation.\n");
            fflush(stdout);
            return 1;
        }
        else if (argv[5][0] == 'c') {
            x_center = entity->x_center;
        }
        errno = 0;
        double y_center = strtod(argv[6], NULL);
        if (errno != 0 && argv[6][0] != 'c') {
            printf("Invalid center of rotation.\n");
            fflush(stdout);
            return 1;
        }
        else if (argv[6][0] == 'c') {
            y_center = entity->y_center;
        }
        errno = 0;
        double z_center = strtod(argv[7], NULL);
        if (errno != 0 && argv[7][0] != 'c') {
            printf("Invalid center of rotation.\n");
            fflush(stdout);
            return 1;
        }
        else if (argv[7][0] == 'c') {
            z_center = entity->z_center;
        }

        uint8_t axis_of_rotation =
            argv[3][0] == 'x'   ? MSGDETAIL_ROTATE_ENTITY_AXIS_X
            : argv[3][0] == 'y' ? MSGDETAIL_ROTATE_ENTITY_AXIS_Y
                                : MSGDETAIL_ROTATE_ENTITY_AXIS_Z;
        broadcast_rotate(space,
                         id,
                         axis_of_rotation,
                         angle_in_radians,
                         x_center,
                         y_center,
                         z_center);
        printf("rotated entity\n");
    } else if (argv[1][0] == 'l') {
        LightSource *light = get_light(space, id);
        if (light == NULL) {
            printf("unknown light\n");
            fflush(stdout);
            return 1;
        }
        errno = 0;
        double x_center = strtod(argv[5], NULL);
        if (errno != 0 && argv[5][0] != 'c') {
            printf("Invalid center of rotation.\n");
            fflush(stdout);
            return 1;
        }
        else if (argv[5][0] == 'c') {
            x_center = light->x;
        }
        errno = 0;
        double y_center = strtod(argv[6], NULL);
        if (errno != 0 && argv[6][0] != 'c') {
            printf("Invalid center of rotation.\n");
            fflush(stdout);
            return 1;
        }
        else if (argv[6][0] == 'c') {
            y_center = light->y;
        }
        errno = 0;
        double z_center = strtod(argv[7], NULL);
        if (errno != 0 && argv[7][0] != 'c') {
            printf("Invalid center of rotation.\n");
            fflush(stdout);
            return 1;
        }
        else if (argv[7][0] == 'c') {
            z_center = light->z;
        }
        uint8_t axis_of_rotation =
            argv[3][0] == 'x'   ? MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_X
            : argv[3][0] == 'y' ? MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_Y
                                : MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_Z;
        broadcast_rotate_light(space,
                               id,
                               axis_of_rotation,
                               angle_in_radians,
                               x_center,
                               y_center,
                               z_center);
        printf("rotated light\n");
    } else {
        printf("You must specify 'e' or 'l' for entity or light.\n");
        fflush(stdout);
        return 1;
    }

    fflush(stdout);
    return 0;
}

int __handle_command__create(int argc, char **argv) {
    if (argc < 2) {
        printf("Syntax for entity: create <e[ntity]> <id> <x_corner> <y_corner> <z_corner> <x_length> <y_length> <z_length>\n");
        printf("Syntax for light : create <l[ight]> <id> <x_coord> <y_coord> <z_coord> <intensity>\n");
        fflush(stdout);
        return 1;
    }
    if (argv[1][0] == 'e' && argc < 9) {
        printf("hi\n");
        printf("Syntax for entity: create <e[ntity]> <id> <x_corner> <y_corner> <z_corner> <x_length> <y_length> <z_length>\n");
        printf("Syntax for light : create <l[ight]> <id> <x_coord> <y_coord> <z_coord> <intensity>\n");
        fflush(stdout);
        return 1;
    }
    if (argv[1][0] == 'l' && argc < 7) {
        printf("Syntax for entity: create <e[ntity]> <id> <x_corner> <y_corner> <z_corner> <x_length> <y_length> <z_length>\n");
        printf("Syntax for light : create <l[ight]> <id> <x_coord> <y_coord> <z_coord> <intensity>\n");
        fflush(stdout);
        return 1;
    }

    errno = 0;
    int id = (int)strtol(argv[2], NULL, 10);
    if (errno != 0) {
        printf("You must specify a numeric ID.\n");
        fflush(stdout);
        return 1;
    }
    if (argv[1][0] == 'e' && (id < 0 || id >= MAX_ENTITIES)) {
        printf("invalid id\n");
        fflush(stdout);
        return 1;
    }
    if (argv[1][0] == 'l' && (id < 0 || id >= MAX_LIGHTS)) {
        printf("invalid id\n");
        fflush(stdout);
        return 1;
    }

    errno = 0;
    double x_coord = strtod(argv[3], NULL);
    if (errno != 0) {
        printf("Invalid x coordinate.\n");
        fflush(stdout);
        return 1;
    }
    errno = 0;
    double y_coord = strtod(argv[4], NULL);
    if (errno != 0) {
        printf("Invalid y coordinate.\n");
        fflush(stdout);
        return 1;
    }
    errno = 0;
    double z_coord = strtod(argv[5], NULL);
    if (errno != 0) {
        printf("Invalid z coordinate.\n");
        fflush(stdout);
        return 1;
    }

    if (argv[1][0] == 'e') {

        errno = 0;
        double x_len = strtod(argv[6], NULL);
        if (errno != 0) {
            printf("Invalid x side length.\n");
            fflush(stdout);
            return 1;
        }
        errno = 0;
        double y_len = strtod(argv[7], NULL);
        if (errno != 0) {
            printf("Invalid y side length.\n");
            fflush(stdout);
            return 1;
        }
        errno = 0;
        double z_len = strtod(argv[8], NULL);
        if (errno != 0) {
            printf("Invalid z side length.\n");
            fflush(stdout);
            return 1;
        }
        EntitySpace *space = get_space();
        broadcast_create_entity(space, id, x_coord, y_coord, z_coord, x_len, y_len, z_len);
        printf("created entity\n");
        return 0;
    }

    else if (argv[1][0] == 'l') {
        errno = 0;
        double intensity = strtod(argv[6], NULL);
        if (errno != 0) {
            printf("Invalid intensity\n");
            fflush(stdout);
            return 1;
        }

        EntitySpace *space = get_space();
        broadcast_create_light_source(space, id, x_coord, y_coord, z_coord, intensity);
        printf("created light\n");
        return 0;
    }

    else {
        printf("You must specify 'e' or 'l' for entity or light.\n");
        fflush(stdout);
        return 1;
    }
}

CommandHandler __command_handlers[NUM_AVAILABLE_COMMAND_HANDLERS] = {
    (CommandHandler){
        .command_name = "exit",
        .handle_command = __handle_command__exit,
    },
    (CommandHandler){
        .command_name = "list",
        .handle_command = __handle_command__list,
    },
    (CommandHandler){
        .command_name = "delete",
        .handle_command = __handle_command__delete,
    },
    (CommandHandler){
        .command_name = "translate",
        .handle_command = __handle_command__translate,
    },
    (CommandHandler){
        .command_name = "brighten",
        .handle_command = __handle_command__brighten,
    },
    (CommandHandler){
        .command_name = "rotate",
        .handle_command = __handle_command__rotate,
    },
    (CommandHandler){
        .command_name = "create",
        .handle_command = __handle_command__create,
    }
};

CommandHandler *get_command_handler(char *command_name) {
    CommandHandler *found_handler = NULL;

    for (int i = 0; i < NUM_AVAILABLE_COMMAND_HANDLERS; i++) {
        if (strcmp(__command_handlers[i].command_name, command_name) == 0) {
            found_handler = &(__command_handlers[i]);
            break;
        }
    }

    return found_handler;
}
