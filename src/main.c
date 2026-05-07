#include <string.h>
#include "buffer.h"
#include "term.h"
#include "util.h"
#include "view.h"

void draw_view(View *v, byte *buffer) {
    RenderView rv = view_get_render_view(v);
    for (usize i = 0; i < rv.size; i++) {
        buffer[i] = rv.cells[i].c;
    }

    term_write(STRING(buffer, rv.size));
}

int main(void) {
    Size size = term_get_size();
    if (size.width == 0)
        PANIC("failed to get terminal size");

    term_new_buffer();

    #define MAX_BUFFER (1024 * 100)
    byte buffer[MAX_BUFFER];
    memset(buffer, ' ', MAX_BUFFER);

    View *view = view_create(size.width, size.height);
    Buffer *b = buffer_create();

    while (true) {
        Input input = term_get_input();
        if (input.control == CONTROL_UP)
            break;

        if (input.control == CONTROL_NONE)
            buffer_write(b, STRING(&input.character, 1));

        if (input.control == CONTROL_LEFT)
            cursor_move(b, -1, 0);
        if (input.control == CONTROL_RIGHT)
            cursor_move(b, 1, 0);
        if (input.control == CONTROL_ENTER) {
            buffer_insert_line(b, (int)cursor_get_pos(b).row+1);
            cursor_move(b, 0, 1);
        }

        buffer_render(b, view);
        draw_view(view, buffer);
        term_set_cursor_pos(0, 0);
    }

    buffer_destroy(b);
    term_restore_buffer();
    return 0;
}
