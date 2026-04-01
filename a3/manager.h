#ifndef MANAGER_H
#define MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include "space.h"
#include "renderer.h"
#define NUM_WORKERS (8)


EntitySpace *get_space();
int *get_write_fds();
DistanceMap get_map();

void broadcast_to_pipes(void *source_buffer, size_t nbytes);
void broadcast_translate(EntitySpace *space, int entity_id, double x_offset, double y_offset, double z_offset);
void broadcast_rotate(EntitySpace *space, int entity_id, uint8_t axis_of_rotation, double angle, double x_center, double y_center, double z_center);
void broadcast_translate_light(EntitySpace *space, int entity_id, double x_offset, double y_offset, double z_offset);
void broadcast_rotate_light(EntitySpace *space, int entity_id, uint8_t axis_of_rotation, double angle, double x_center, double y_center, double z_center);
void broadcast_brighten_light(EntitySpace *space, int entity_id, double delta_intensity);
Entity *broadcast_create_entity(EntitySpace *space, int entity_id, double x_corner, double y_corner, double z_corner, double x_length, double y_length, double z_length);
LightSource *broadcast_create_light_source(EntitySpace *space, int entity_id, double x, double y, double z, double intensity);
void broadcast_delete_entity(EntitySpace *space, int entity_id);
void broadcast_delete_light_source(EntitySpace *space, int entity_id);

void set_stdin_back_to_user_terminal();

#endif
