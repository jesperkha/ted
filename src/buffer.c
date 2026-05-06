#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "buffer.h"
#include "util.h"

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
                .text = STRING(ptr, length),
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
            .text = STRING(malloc(DEFAULT_LINE_LENGTH), 0),
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
    return b;
}

void buffer_destroy(Buffer *b) {
    free(b);
}
