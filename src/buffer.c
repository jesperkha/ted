#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "buffer.h"
#include "util.h"
#include "view.h"

#define LINEBUF_DEFAULT_SIZE 128
#define DEFAULT_LINE_LENGTH 32

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
        count = 1;
        lines[0] = (Line){
            .text = malloc(DEFAULT_LINE_LENGTH),
            .length = 0,
            .cap = DEFAULT_LINE_LENGTH,
            .err = false,
            .written = true,
        };
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

    usize sum_bytes = 0;

    for (usize i = 0; i < line_count; i++) {
        usize buffer_line_number = i + b->line_offset;
        Line line = b->lines.lines[buffer_line_number];
        sum_bytes += view_write_line(view, STRING(line.text, line.length), i);
    }

    return sum_bytes;
}

void buffer_write(Buffer *b, String text) {
    Line line = b->lines.lines[b->cursor.row];
    if (!line.written) {
        TODO("fresh line alloc not implemented");
    }

    if (text.length + b->cursor.col > line.cap) {
        TODO("line realloc not implemented");
    }

    memcpy(line.text + b->cursor.col, text.s, text.length);
    line.length += text.length;
}
