#pragma once

#include "util.h"
#include <stdint.h>

// A Cell is a single character drawn to the screen. It contains the character
// itself, styling information, and additional flags.
typedef struct Cell {
    uint32_t fg, bg;
    uint16_t flags;
    byte c;
} Cell;

typedef struct View View;

// Create a new root view. Initialize empty Cell buffer.
View *view_create(usize width, usize height);

// Create a view into parent with the given dimensions.
View *view_from(View *parent, usize x, usize y, usize w, usize h);

// Write count cells to a line in v. Will at most write the number of cells
// equal to the view width. Returns number of cells written.
usize view_write_line(View *v, Cell *cells, usize count, usize line);

// Get the size of v.
Size view_size(View *v);

// RenderView is a type representing the necessary components of a View
// needed to be rendered to the screen.
typedef struct RenderView {
    Cell *cells;
    usize size;
} RenderView;

// Get the RenderView from v. This is used to actually draw its contents.
RenderView view_get_render_view(View *v);
