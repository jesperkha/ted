#include "view.h"

struct View {
    // Parent view if any.
    View *parent;
    // Buffer points to the first byte in this view.
    byte *buffer;
    usize width, height;
    // Number of bytes from line start to next line start.
    usize line_wrap_amount;
};
