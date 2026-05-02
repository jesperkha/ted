#include <stdbool.h>
#include <stdint.h>
#include "strbuf.h"
#include "buffer.h"
#include "util.h"

struct Buffer {
    Cursor cursor;
};

Buffer *buffer_create(void)
{
    TODO("buffer create");
}

void buffer_destroy(Buffer *b)
{
    TODO("buffer destroy");
}

void buffer_write(Buffer *b, StrBuf text)
{
    TODO("buffer write");
}

void buffer_overwrite(Buffer *b, StrBuf text)
{
    TODO("buffer overwrite");
}

void buffer_delete(Buffer *b, unsigned int count, DeleteDirection dir)
{
    TODO("buffer delete");
}

int buffer_insert_line(Buffer *b, int after)
{
    TODO("buffer insert line");
}

bool buffer_delete_line(Buffer *b, unsigned int ln)
{
    TODO("buffer delete line");
}

StrBuf buffer_read_line(Buffer *b, unsigned int ln)
{
    TODO("buffer read line");
}

bool cursor_set_pos(Buffer *b, unsigned int row, unsigned int col)
{
    TODO("cursor set pos");
}

bool cursor_move(Buffer *b, int x, int y)
{
    TODO("cursor move");
}

Cursor cursor_get_pos(Buffer *b)
{
    TODO("cursor get pos");
}
