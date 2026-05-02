#pragma once

#include "strbuf.h"
#include <stdbool.h>

typedef enum ControlKey {
   CONTROL_NONE,
   CONTROL_CTRL,
   CONTROL_ALT,
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

// Set terminal raw mode according to [enable].
void term_set_raw_mode(bool enable);

// Get terminal input in raw mode.
Input term_get_input(void);

// Set the terminal cursor position.
void term_set_cursor_pos(unsigned int row, unsigned int col);

typedef struct TermSize
{
   unsigned int rows;
   unsigned int cols;
} TermSize;

// Get the terminal buffer size in rows and columns.
TermSize term_get_size(void);

// Write [buf] to the terminal at the cursor position.
void term_write(StrBuf buf);
