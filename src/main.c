#include "buffer.h"
#include "term.h"
#include "util.h"
#include "view.h"

int main(void) {
    Size size = term_get_size();
    if (size.width == 0)
        PANIC("failed to get terminal size");

    term_new_buffer();

    #define MAX_BUFFER (1024 * 100)
    byte buffer[MAX_BUFFER];

    View *view = view_create(size.width, size.height);
    View *child = view_from(view, 10, 10, 10, 10);
    Buffer *b = buffer_create();

    while (true) {
        Input input = term_get_input();
        if (input.control == CONTROL_ESC)
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

        buffer_render(b, child);
        usize count = view_render(view, buffer, MAX_BUFFER);
        term_write(STRING(buffer, count));
        term_set_cursor_pos(0, 0);
    }

    buffer_destroy(b);
    term_restore_buffer();
    return 0;
}
