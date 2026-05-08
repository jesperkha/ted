#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "color.h"
#include "util.h"
#include "view.h"

#define LINEBUF_DEFAULT_SIZE 128
#define DEFAULT_LINE_LENGTH 32

// Global buffer used to draw a single line to before rendering to a view.
Cell line_draw_buffer[512];

typedef struct LineBuffer {
    // Source text if any. May be ERROR_STRING for empty buffer.
    String source;
    Line *lines;
    usize cap;
    usize count;
} LineBuffer;

struct Buffer {
    String filepath;
    LineBuffer lines;
    Cursor cursor;
    usize line_offset;
    bool dirty;
};

// Create new Line with content if not ERROR_STRING.
static Line new_line(String content) {
    if (content.err) {
        byte *ptr = malloc(DEFAULT_LINE_LENGTH);
        if (ptr == NULL)
            PANIC("failed to allocate line");

        return (Line){
            .text = ptr,
            .length = 0,
            .cap = DEFAULT_LINE_LENGTH,
            .err = false,
            .written = true,
        };
    }

    TODO("content line not implemented");
    return (Line){0};
}

static LineBuffer new_line_buffer(String source) {
    Line *lines = malloc(LINEBUF_DEFAULT_SIZE * sizeof(Line));
    if (lines == NULL)
        PANIC("failed to allocate lines");

    usize count = 0;
    usize cap = LINEBUF_DEFAULT_SIZE;

    // If source is given, collect all line beginnings in source.
    if (!source.err) {
        usize line_start = 0;

        for (usize i = 0; i < source.length; i++) {
            // Reallocate line array if too small
            if (count >= cap) {
                cap *= 2;
                lines = realloc(lines, cap * sizeof(Line));
                if (lines == NULL)
                    PANIC("failed to reallocate lines");
            }

            char c = source.s[i];
            if (c != '\n')
                continue;

            const char *ptr = source.s + line_start;
            usize length = max(i - line_start - 1, 0); // -1 for newline

            Line line = (Line){
                .text = (byte *)ptr,
                .length = length,
                .cap = length,
                .written = false,
                .err = false,
            };

            lines[count++] = line;
            line_start = i + 1;
        }
    }

    // On empty file/buffer, create at least one line.
    if (count == 0) {
        lines[count++] = new_line(ERROR_STRING);
    }

    LineBuffer buf = {
        .source = source,
        .lines = lines,
        .cap = cap,
        .count = count,
    };

    return buf;
}

Buffer *buffer_create(void) {
    Buffer *b = malloc(sizeof(Buffer));
    b->lines = new_line_buffer(ERROR_STRING);
    b->filepath = ERROR_STRING;
    b->dirty = false;
    b->line_offset = 0;
    b->cursor = (Cursor){.col = 0, .row = 0};
    return b;
}

void buffer_destroy(Buffer *b) {
    free(b);
}

int buffer_render(Buffer *b, View *view) {
    Size size = view_size(view);
    usize line_count = min(size.height, b->lines.count);

    usize sum_cells = 0;

    for (usize i = 0; i < line_count; i++) {
        usize buffer_line_number = i + b->line_offset;
        Line line = b->lines.lines[buffer_line_number];

        usize count = min(line.length, size.width);
        Cell *cells = line_draw_buffer;

        for (usize i = 0; i < count; i++) {
            cells[i] = (Cell){.c = line.text[i], .fg = FG_WHITE, .bg = BG_BLACK};
        }

        sum_cells += view_write_line(view, cells, count, i);
    }

    return sum_cells;
}

void buffer_write(Buffer *b, String text) {
    Line *line = &b->lines.lines[b->cursor.row];
    if (!line->written) {
        TODO("fresh line alloc not implemented");
    }

    // Reallocate line if too big
    if (line->length + text.length > line->cap) {
        // TODO: reallocate larger size if text length exceeds new cap
        line->cap *= 2;
        byte *newptr = realloc(line->text, line->cap);
        if (newptr == NULL)
            PANIC("failed to reallocate line");

        line->text = newptr;
    }

    // Move text to the right if cursor is in middle of line
    if (b->cursor.col < line->length) {
        byte *pos = line->text + b->cursor.col;
        memmove(pos + text.length, pos, line->length - b->cursor.col);
    }

    memcpy(line->text + b->cursor.col, text.s, text.length);
    line->length += text.length;
    b->cursor.col += text.length;
}

int buffer_insert_line(Buffer *b, int at) {
    if (at < 0)
        at = b->lines.count;

    LOGF("inserting line at %d", at);

    if (b->lines.count + 1 >= b->lines.cap) {
        b->lines.cap *= 2;
        b->lines.lines = realloc(b->lines.lines, b->lines.cap * sizeof(Line));
        if (b->lines.lines == NULL)
            PANIC("failed to reallocate lines");
    }

    if ((usize)at < b->lines.count) {
    }

    b->lines.lines[at] = new_line(ERROR_STRING);
    b->lines.count++;
    return (int)at;
}

Cursor cursor_get_pos(Buffer *b) {
    return b->cursor;
}

bool cursor_move(Buffer *b, int dx, int dy) {
    int x = (int)b->cursor.col + dx;
    int y = (int)b->cursor.row + dy;

    if (y < 0 || y >= (int)b->lines.count || x < 0)
        return false;

    if (dy == 0 && x > (int)b->lines.lines[y].length)
        return false;

    if (dy != 0) {
        Line newline = b->lines.lines[y]; // asserted not out of bounds
        if (x > (int)newline.length)
            x = newline.length;
    }

    // Both asserted not <0
    b->cursor.col = x;
    b->cursor.row = y;
    return true;
}
