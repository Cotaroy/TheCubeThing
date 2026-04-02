#ifndef RENDERER_H
#define RENDERER_H

#include <stdio.h>
#include "ansi_escape_sequences.h"

#define MAX_DIST 128.0
#define MIN_DIST 0.0
#define MAX_LIGHT 200.0
#define MIN_LIGHT 0.0
#define EPSILON 1e-8

typedef struct {
    int width;
    int height;
    double* distances; // array of length width*height, stored in LTR,TTB reading direction
} DistanceMap;

void render(DistanceMap *map);
void render_luminosity(DistanceMap *map);

/**
 * Print an escape sequence to STDOUT to make the user's terminal
 * clear its screen.
 */
static inline void terminal_clear_screen() { printf(ESC_CLEAR_SCREEN); }
/**
 * Print an escape sequence to STDOUT to make the user's terminal
 * place the cursor in the top left of the window,
 * which means if more stuff is printed hereon, it would write
 * from the topleft corner (possibly overwriting whatever was already there).
 */
static inline void terminal_move_cursor_to_topleft() { printf(ESC_MOVE_CURSOR_TO_TOPLEFT); }
/**
 * Print an escape sequence to STDOUT to make the user's terminal
 * go into an alternate screen (like `vim`/`nano`/`man`) and allow us
 * to restore the original terminal history later.
 */
static inline void terminal_enter_alt_screen() {
    printf(ESC_ALT_SCREEN_BUFFER_ENABLE);
    printf(ESC_MAKE_CURSOR_INVISIBLE);
}
/**
 * Print an escape sequence to STDOUT to make the user's terminal
 * restore the original screen from before we entered this screen.
 */
static inline void terminal_exit_alt_screen() {
    printf(ESC_ALT_SCREEN_BUFFER_DISABLE);
    printf(ESC_MAKE_CURSOR_VISIBLE);
}

#endif
