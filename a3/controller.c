#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include "manager.h"
#include "math.h"
#include "controller.h"

void restore_original_settings(struct termios *og_settings) {
    tcsetattr(STDIN_FILENO, TCSANOW, og_settings);
}

void setup_non_canonical(struct termios *og_settings) {
    tcgetattr(STDIN_FILENO, og_settings);

    struct termios new_settings = *og_settings;

    // disable all of these flags (bitwise or to conjoin the settings, not to make bitwise and turn them off)
    new_settings.c_lflag &= ~(ICANON); 

    new_settings.c_cc[VMIN] = 0;
    // in deciseconds
    new_settings.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
}

void handle_non_canonical_input(double *camera_x, double *camera_y, double *camera_z, double *camera_forward_azimuth, double *camera_forward_inclination) {
    char input_char = 'x';
    int read_res;
    while ((read_res = read(STDIN_FILENO, &input_char, 1)) > 0) {
        if (read_res == -1) {
            perror("failed to read user movement input");
            exit(1);
        }
        if (input_char == 'w') {
            *camera_x += cos(*camera_forward_azimuth) * sin(*camera_forward_inclination) * STEP_DISTANCE;
            *camera_y += sin(*camera_forward_azimuth) * sin(*camera_forward_inclination) * STEP_DISTANCE;
            *camera_z += cos(*camera_forward_inclination) * STEP_DISTANCE;
        }
    }
}
