#pragma once

#include "util.h"

typedef struct View View;

// Create a new root view from a buffer with a set size.
View *view_create(byte *buffer, usize width, usize height);

// Create a view into [parent] with the given dimensions.
View *view_from(View *parent, usize x, usize y, usize w, usize h);

// Write [content] to a [line] in [v]. Any text longer than the view width
// will not be written. Returns number of bytes written.
usize view_write_line(View *v, String content, usize line);
