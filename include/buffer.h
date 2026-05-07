#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "util.h"
#include "view.h"

// Buffer represents a text buffer in the editor.
typedef struct Buffer Buffer;

// Allocate and initialize a new Buffer.
Buffer *buffer_create(void);

// Free Buffer and its contents.
void buffer_destroy(Buffer *b);

// Write text at cursor position, moving all text after the cursor to the right.
void buffer_write(Buffer *b, String text);

// Write text at cursor position, overwriting any text to the right.
void buffer_overwrite(Buffer *b, String text);

typedef enum DeleteDirection {
    DELETE_LEFT,
    DELETE_RIGHT,
} DeleteDirection;

// Delete count characters to the left/right of the cursor.
// dir can be DELETE_LEFT or DELETE_RIGHT.
void buffer_delete(Buffer *b, usize count, DeleteDirection dir);

// Insert a new line at the given line number. Returns the new line number.
// At may be -1 to append the line after the last line in the buffer.
// Returns -1 if insertion failed (line is out of bounds).
int buffer_insert_line(Buffer *b, int at);

// Delete line number ln, moving all trailing lines up one. Returns false if
// ln is out of bounds or the last line in the buffer, in which case it cannot
// be deleted. Returns true on success.
bool buffer_delete_line(Buffer *b, usize ln);

typedef struct Line {
    // Text content of this line.
    byte *text;
    usize length, cap;

    // If this line has been written to or points to the original file.
    // Text content should not be modified if this is false.
    bool written;
    // Invalid Line value.
    bool err;
} Line;

#define ERROR_LINE ((Line){.err = true})

// Read the line number ln. Returns ERROR_LINE if ln is out of bounds.
Line buffer_read_line(Buffer *b, usize ln);

// Render buffer contents to view. Returns number of cells written.
// Returns -1 on error.
int buffer_render(Buffer *b, View *view);

// Cursor is a row/col position in a Buffer. It is guaranteed to be within the
// bounds of the buffer and current line.
typedef struct Cursor {
    usize row;
    usize col;
} Cursor;

// Set cursor position at given row/col. Returns false if out of bounds,
// otherwise true.
bool cursor_set_pos(Buffer *b, usize row, usize col);

// Move the cursor x and y characters in the horizontal and vertical
// direction respectively. Returns false if resulting position is out of
// bounds, in which case the cursor wil not move.
bool cursor_move(Buffer *b, int x, int y);

// Get the current position of the cursor.
Cursor cursor_get_pos(Buffer *b);
