#include "view.h"
#include "util.h"
#include <string.h>

struct View {
    // Parent view if any.
    View *parent;
    // Buffer points to the first byte in this view.
    byte *buffer;
    usize width, height;
    // Number of bytes from line start to next line start.
    usize line_wrap_amount;
};

View *view_create(byte *buffer, usize width, usize height) {
    View *v = malloc(sizeof(View));
    v->parent = NULL;
    v->buffer = buffer;
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

usize view_write_line(View *v, String content, usize line) {
    if (content.err || v->buffer == NULL)
        return 0;

    if (line > v->height)
        return 0;

    usize write_count = min(v->width, content.length);
    byte *offset = v->buffer + (line * v->line_wrap_amount);
    memcpy(offset, content.s, write_count);
    return write_count;
}

String view_to_string(View *v) {
    return STRING((char *)v->buffer, v->width * v->height);
}

Size view_size(View *v) {
    return SIZE(v->width, v->height);
}
