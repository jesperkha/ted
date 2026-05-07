#include "view.h"
#include "util.h"
#include <string.h>

struct View {
    // Parent view if any.
    View *parent;
    // Buffer points to the first cell in the view.
    Cell *buffer;
    usize width, height;
    // Number of cells from line start to next line start.
    usize line_wrap_amount;
};

View *view_create(usize width, usize height) {
    View *v = malloc(sizeof(View));
    v->parent = NULL;
    v->buffer = malloc(sizeof(Cell) * width * height);
    if (v->buffer == NULL)
        PANIC("failed to allocate view buffer");

    // TODO: view destroy

    // Initialize clear buffer
    for (usize i = 0; i < width * height; i++) {
        v->buffer[i].c = ' ';
    }

    v->width = width;
    v->height = height;
    v->line_wrap_amount = width;
    return v;
}

View *view_from(View *parent, usize x, usize y, usize w, usize h) {
    View *v = malloc(sizeof(View));
    v->parent = parent;
    v->line_wrap_amount = parent->line_wrap_amount;

    usize offset = (y * parent->line_wrap_amount) + x;
    v->buffer = parent->buffer + offset;

    v->width = w;
    v->height = h;
    return v;
}

usize view_write_line(View *v, Cell *cells, usize count, usize line) {
    if (cells == NULL || v->buffer == NULL || count == 0)
        return 0;

    if (line >= v->height)
        return 0;

    usize write_count = min(v->width, count);
    Cell *offset = v->buffer + (line * v->line_wrap_amount);
    memcpy(offset, cells, write_count * sizeof(Cell));
    return write_count;
}

Size view_size(View *v) {
    return SIZE(v->width, v->height);
}

RenderView view_get_render_view(View *v) {
    return (RenderView){.cells = v->buffer, .size = v->width * v->height};
}
