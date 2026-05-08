#include "view.h"
#include "color.h"
#include "util.h"
#include <string.h>

#define LINE_DRAW_BUFFER_SIZE 512

// Global buffer used to draw a single line to before rendering to a view.
Cell line_draw_buffer[LINE_DRAW_BUFFER_SIZE];

Cell *view_get_line_buffer(usize *size) {
    if (size != NULL) {
        *size = LINE_DRAW_BUFFER_SIZE;
    }
    return line_draw_buffer;
}

struct View {
    // Parent view if any.
    View *parent;
    // Buffer points to the first cell in the view.
    Cell *buffer;
    usize x, y, width, height;
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
        v->buffer[i].bg = BG_NONE;
        v->buffer[i].fg = FG_WHITE;
        v->buffer[i].c = ' ';
    }

    v->x = v->y = 0;
    v->width = width;
    v->height = height;
    v->line_wrap_amount = width;
    return v;
}

static void init_view(View *parent, View *v, usize x, usize y, usize w, usize h) {
    v->parent = parent;
    v->line_wrap_amount = parent->line_wrap_amount;

    usize offset = (y * parent->line_wrap_amount) + x;
    v->buffer = parent->buffer + offset;

    if (x > parent->width || y > parent->height)
        PANIC("position of child view is outside parent");

    // Clamp width and height to be contained in parent.
    if (x + w > parent->width)
        w = parent->width - x;
    if (y + h > parent->height)
        h = parent->height - y;

    v->x = x;
    v->y = y;
    v->width = w;
    v->height = h;
}

View *view_from(View *parent, usize x, usize y, usize w, usize h) {
    if (parent == NULL)
        return NULL;

    View *v = malloc(sizeof(View));
    init_view(parent, v, x, y, w, h);
    return v;
}

usize view_write_line(View *v, Cell *cells, usize x, usize y, usize count) {
    if (v == NULL || cells == NULL || v->buffer == NULL || count == 0)
        return 0;

    if (y >= v->height || x >= v->width)
        return 0;

    usize write_count = min(min(v->width, count), v->width - x);
    Cell *offset = v->buffer + x + (y * v->line_wrap_amount);
    memcpy(offset, cells, write_count * sizeof(Cell));
    return write_count;
}

Size view_size(View *v) {
    if (v == NULL)
        return SIZE(0, 0);
    return SIZE(v->width, v->height);
}

usize view_render(View *v, byte *buffer, usize max_size) {
    if (v == NULL)
        return 0;

    usize cell_size = 5 + 5 + 1; // Two colors and the character

    // Number of bytes and cells to write
    usize byte_count = min(max_size, v->width * v->height * cell_size);
    usize cell_count = byte_count / cell_size;

    usize n = 0; // Byte position
    for (usize i = 0; i < cell_count; i++) {
        Cell c = v->buffer[i];

        memcpy(buffer+n, "\x1b[", 2);
        n += 2;
        buffer[n++] = c.bg >> 8;
        buffer[n++] = c.bg & 0xff;
        buffer[n++] = 'm';

        memcpy(buffer+n, "\x1b[", 2);
        n += 2;
        buffer[n++] = c.fg >> 8;
        buffer[n++] = c.fg & 0xff;
        buffer[n++] = 'm';

        buffer[n++] = c.c;
    }

    return byte_count;
}

bool view_resize(View *v, int dx, int dy, int dw, int dh) {
    if (v == NULL || v->parent == NULL)
        return false;

    int x = v->x + dx;
    int y = v->y + dy;
    int w = v->width + dw;
    int h = v->height + dh;

    if (x < 0 || y < 0 || w < 0 || h < 0)
        return false;

    init_view(v->parent, v, x, y, w, h);
    return true;
}

bool view_set_size(View *v, usize x, usize y, usize w, usize h) {
    if (v == NULL || v->parent == NULL)
        return false;

    init_view(v->parent, v, x, y, w, h);
    return true;
}

usize view_clear(View *v, uint16_t color) {
    if (v == NULL)
        return 0;

    for (usize i = 0; i < v->height; i++) {
        usize size;
        Cell *cells = view_get_line_buffer(&size);
        usize count = min(size, v->width);

        for (usize i = 0; i < count; i++) {
            cells[i] = (Cell){.c = ' ', .fg = FG_NONE, .bg = color};
        }

        view_write_line(v, cells, 0, i, count);
    }

    return v->width * v->height;
}

Cell *view_cell_at(View *v, usize x, usize y) {
    if (v == NULL || x >= v->width || y >= v->height)
        return NULL;

    return &v->buffer[x + (y * v->line_wrap_amount)];
}
