#pragma once

#include "util.h"
#include <stdint.h>

// A Cell is a single character drawn to the screen. It contains the character
// itself, styling information, and additional flags.
typedef struct Cell {
    uint16_t fg, bg;
    uint16_t flags;
    byte c;
} Cell;

typedef struct View View;

// Create a new root view. Initialize empty Cell buffer.
View *view_create(usize width, usize height);

// Create a view into parent with the given dimensions.
View *view_from(View *parent, usize x, usize y, usize w, usize h);

// Resize the view with delta x,y,w,h. May clamp values based on parent size.
// Returns true on success;
bool view_resize(View *v, int dx, int dy, int dw, int dh);

// Sets the position and and size of the view. Returns true on success.
bool view_set_size(View *v, usize x, usize y, usize w, usize h);

// Get a buffer to render to before writing line to a view. The size of the
// buffer in cells is written to size if not null.
Cell *view_get_line_buffer(usize *size);

// Write count cells to a line y in v at offset x. Will at most write the number
// of cells equal to the view width-x. Returns number of cells written.
usize view_write_line(View *v, Cell *cells, usize x, usize y, usize count);

// Get the cell at x,y. Returns NULL if out of bounds.
Cell *view_cell_at(View *v, usize x, usize y);

// Get the size of v.
Size view_size(View *v);

// Render v to a buffer with a maximum byte length. Returns number of bytes written.
usize view_render(View *v, byte *buffer, usize max_size);

// Fill the entire view with the given background color.
// Returns number of cells written.
usize view_clear(View *v, uint16_t color);
