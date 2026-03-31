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

// int __handle_command__blank(int argc, char **argv) {
//     // suppress unused variable warnings
//     // because actually just don't need these
//     (void)argc;
//     (void)argv;

//     // exit_line_command_mode();
//     setup_non_canonical();
//     return 0;
// }

int __handle_command__exit(int argc, char **argv) {
    // suppress unused variable warnings
    // because actually just don't need these
    (void)argc;
    (void)argv;

    printf("byebye\n");
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

int __handle_command__rotate(int argc, char **argv) {
    if (argc < 5) {
        printf("Syntax: %s <e[ntity]|l[ight]> <id> <x|y|z (axis of rotation)> "
               "<angle in degrees counterclockwise>\n",
               argv[0]);
        fflush(stdout);
        return 1;
    }

    errno = 0;
    int id = (int)strtol(argv[2], NULL, 10);
    if (errno != 0) {
        printf("You must specify a numeric ID.");
        fflush(stdout);
        return 1;
    }
    if (argv[1][0] == 'e' && (id < 0 || id >= MAX_ENTITIES)) {
        printf("invalid id");
        fflush(stdout);
        return 1;
    }
    if (argv[1][0] == 'l' && (id < 0 || id >= MAX_LIGHTS)) {
        printf("invalid id");
        fflush(stdout);
        return 1;
    }

    if (argv[3][0] != 'x' && argv[3][0] != 'y' && argv[3][0] != 'z') {
        printf("specify x, y, or z as axis of rotation.");
        fflush(stdout);
        return 1;
    }

    errno = 0;
    double angle_in_degrees = strtod(argv[4], NULL);
    if (errno != 0) {
        printf("Invalid angle.");
        fflush(stdout);
        return 1;
    }
    double angle_in_radians = PI * angle_in_degrees / 180;

    EntitySpace *space = get_space();
    if (argv[1][0] == 'e') {
        Entity *entity = get_entity(space, id);
        if (entity == NULL) {
            printf("unknown entity");
            fflush(stdout);
            return 1;
        }

        uint8_t axis_of_rotation =
            argv[3][0] == 'x'   ? MSGDETAIL_ROTATE_ENTITY_AXIS_X
            : argv[3][0] == 'y' ? MSGDETAIL_ROTATE_ENTITY_AXIS_Y
                                : MSGDETAIL_ROTATE_ENTITY_AXIS_Z;
        broadcast_rotate(space,
                         id,
                         axis_of_rotation,
                         angle_in_radians,
                         entity->x_center,
                         entity->y_center,
                         entity->z_center);
        printf("rotated entity");
    } else if (argv[1][0] == 'l') {
        LightSource *light = get_light(space, id);
        if (light == NULL) {
            printf("unknown light");
            fflush(stdout);
            return 1;
        }
        uint8_t axis_of_rotation =
            argv[3][0] == 'x'   ? MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_X
            : argv[3][0] == 'y' ? MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_Y
                                : MSGDETAIL_ROTATE_LIGHTSOURCE_AXIS_Z;
        broadcast_rotate_light(space,
                               id,
                               axis_of_rotation,
                               angle_in_radians,
                               light->x,
                               light->y,
                               light->z);
        printf("rotated light");
    } else {
        printf("You must specify 'e' or 'l' for entity or light.");
        fflush(stdout);
        return 1;
    }

    fflush(stdout);
    return 0;
}

CommandHandler __command_handlers[NUM_AVAILABLE_COMMAND_HANDLERS] = {
    // (CommandHandler){
    //     .command_name = "",
    //     .handle_command = __handle_command__blank,
    // },
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
