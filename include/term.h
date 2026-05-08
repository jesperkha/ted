#pragma once

#include <stdbool.h>
#include "util.h"

typedef enum ControlKey {
   CONTROL_NONE,
   CONTROL_CTRL,
   CONTROL_ALT,
   CONTROL_ESC,
   CONTROL_ENTER,
   CONTROL_BACKSPACE,
   CONTROL_DELETE,
   CONTROL_TAB,
   CONTROL_LEFT,
   CONTROL_RIGHT,
   CONTROL_UP,
   CONTROL_DOWN,
} ControlKey;

typedef struct Input {
   ControlKey control;
   char character;
} Input;

// Set terminal raw mode according to enable.
void term_set_raw_mode(bool enable);

// Get terminal input in raw mode.
Input term_get_input(void);

// Set the terminal cursor position.
void term_set_cursor_pos(usize row, usize col);

// Get the terminal buffer size in rows and columns. Returns zero size on failure.
Size term_get_size(void);

// Write buf to the terminal at the cursor position.
void term_write(String buf);

// Clears terminal buffer.
void term_clear(void);

// Toggle cursor visibility.
void term_set_cursor_visible(bool visible);

// Create new alternate buffer to draw to and restore to later.
void term_new_buffer(void);

// Restore to previous buffer.
void term_restore_buffer(void);
