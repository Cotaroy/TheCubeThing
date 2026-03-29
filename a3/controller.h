#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <termios.h>
#include <unistd.h>

#define STEP_DISTANCE 0.2

void restore_original_settings(struct termios *og_settings);

void setup_non_canonical(struct termios *og_settings);

// function for handling movement of the camera
// precondition: setup_non_canonical was called earlier
void handle_non_canonical_input(double *camera_x, double *camera_y, double *camera_z, double *camera_forward_azimuth, double *camera_forward_inclination);

#endif
