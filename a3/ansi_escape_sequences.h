#ifndef ANSI_ESCAPE_SEQUENCES_H
#define ANSI_ESCAPE_SEQUENCES_H

/**
 * Escape sequences to manipulate the user's terminal display.
 * 
 * Gathered from
 * - https://en.wikipedia.org/wiki/ANSI_escape_code
 * - https://gist.github.com/ConnerWill/d4b6c776b509add763e17f9f113fd25b
 */

#define ESC_CLEAR_SCREEN "\x1b[2J"
#define ESC_MOVE_CURSOR_TO_TOPLEFT "\x1b[H"
#define ESC_ALT_SCREEN_BUFFER_ENABLE "\x1b[?1049h" // makes the terminal start printing in a new buffer
#define ESC_ALT_SCREEN_BUFFER_DISABLE "\x1b[?1049l" // restores whatever was in the terminal before
#define ESC_MAKE_CURSOR_VISIBLE "\x1b[?25h"
#define ESC_MAKE_CURSOR_INVISIBLE "\x1b[?25l"

#endif
