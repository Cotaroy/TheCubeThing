#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include "manager.h"
#include "math.h"
#include "controller.h"
#define PI (3.14159265358979323846)

struct termios og_settings;

void restore_original_settings() {
    tcsetattr(STDIN_FILENO, TCSANOW, &og_settings);
}

void setup_non_canonical() {
    tcgetattr(STDIN_FILENO, &og_settings);

    struct termios new_settings = og_settings;

    // disable all of these flags (bitwise or to conjoin the settings, not to make bitwise and turn them off)
    new_settings.c_lflag &= ~(ICANON); 

    new_settings.c_cc[VMIN] = 0;
    // in deciseconds
    new_settings.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);
}

void enter_line_command_mode() {
    struct termios current_settings;
    tcgetattr(STDIN_FILENO, &current_settings);

    // turn on canonical
    current_settings.c_lflag |= ICANON;

    tcsetattr(STDIN_FILENO, TCSANOW, &current_settings);
}

void exit_line_command_mode() {
    struct termios current_settings;
    tcgetattr(STDIN_FILENO, &current_settings);

    current_settings.c_lflag &= ~ICANON;
    current_settings.c_cc[VMIN] = 0;
    current_settings.c_cc[VTIME] = 0;
}

void handle_non_canonical_input(double *camera_x, double *camera_y, double *camera_z, double *camera_forward_azimuth, double *camera_forward_inclination) {
    char input_char = 'x';
    int read_res;
    while ((read_res = read(STDIN_FILENO, &input_char, 1)) > 0) {
        if (read_res == -1) {
            perror("failed to read user movement input");
            exit(1);
        }
        switch (input_char) {
            case 'w':
                *camera_x += cos(*camera_forward_azimuth) * sin(*camera_forward_inclination) * STEP_DISTANCE;
                *camera_y += sin(*camera_forward_azimuth) * sin(*camera_forward_inclination) * STEP_DISTANCE;
                *camera_z += cos(*camera_forward_inclination) * STEP_DISTANCE;
                break;
            case 's':
                *camera_x -= cos(*camera_forward_azimuth) * sin(*camera_forward_inclination) * STEP_DISTANCE;
                *camera_y -= sin(*camera_forward_azimuth) * sin(*camera_forward_inclination) * STEP_DISTANCE;
                *camera_z -= cos(*camera_forward_inclination) * STEP_DISTANCE;
                break;
            case 'a':
                double forward[3] = {cos(*camera_forward_azimuth) * sin(*camera_forward_inclination), 
                                     sin(*camera_forward_azimuth) * sin(*camera_forward_inclination), 
                                     cos(*camera_forward_inclination)};
                double forward_down[3] = {cos(*camera_forward_azimuth) * sin(*camera_forward_inclination + PI/2), 
                                          sin(*camera_forward_azimuth) * sin(*camera_forward_inclination + PI/2), 
                                          cos(*camera_forward_inclination) + PI/2};
                double cross[3] = {forward[1] * forward_down[2] - forward[2] * forward_down[1],
                                   forward[2] * forward_down[0] - forward[0] * forward_down[2],
                                   forward[0] * forward_down[1] - forward[1] * forward_down[0]};
                *camera_x -= cross[0] * STEP_DISTANCE;
                *camera_y -= cross[1] * STEP_DISTANCE;
                *camera_z -= cross[2] * STEP_DISTANCE;
                break;
            case 'd':
                double forward1[3] = {cos(*camera_forward_azimuth) * sin(*camera_forward_inclination), 
                                     sin(*camera_forward_azimuth) * sin(*camera_forward_inclination), 
                                     cos(*camera_forward_inclination)};
                double forward_down1[3] = {cos(*camera_forward_azimuth) * sin(*camera_forward_inclination + PI/2), 
                                          sin(*camera_forward_azimuth) * sin(*camera_forward_inclination + PI/2), 
                                          cos(*camera_forward_inclination) + PI/2};
                double cross1[3] = {forward1[1] * forward_down1[2] - forward1[2] * forward_down1[1],
                                   forward1[2] * forward_down1[0] - forward1[0] * forward_down1[2],
                                   forward1[0] * forward_down1[1] - forward1[1] * forward_down1[0]};
                *camera_x += cross1[0] * STEP_DISTANCE;
                *camera_y += cross1[1] * STEP_DISTANCE;
                *camera_z += cross1[2] * STEP_DISTANCE;
                break;
            case 'h':
                *camera_forward_azimuth += STEP_TURN_DISTANCE_HORI;
                if (*camera_forward_azimuth > 2 * PI) {
                    *camera_forward_azimuth -= 2 * PI;
                }
                break;
            case 'l':
                *camera_forward_azimuth -= STEP_TURN_DISTANCE_HORI;
                if (*camera_forward_azimuth < 0) {
                    *camera_forward_azimuth += 2 * PI;
                }
                break;
            case 'j':
                *camera_forward_inclination += STEP_TURN_DISTANCE_VERTI;
                if (*camera_forward_inclination > PI) *camera_forward_inclination = PI;
                break;
            case 'k':
                *camera_forward_inclination -= STEP_TURN_DISTANCE_VERTI;
                if (*camera_forward_inclination < 0) *camera_forward_inclination = 0;
                break;
            case ':':
                enter_line_command_mode();
                char string[30];
                if (scanf("%s", string) == -1) {
                    perror("scanf");
                    exit(1);
                }
                enter_line_command_mode();
        }
    }
}
