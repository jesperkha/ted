#include <string.h>
#include "term.h"
#include "util.h"
#include "view.h"

int main(void)
{
    TermSize size = term_get_size();
    if (size.cols == 0)
        PANIC("failed to get terminal size");

    term_new_buffer();

    #define MAX_BUFFER (1024 * 100)
    byte buffer[MAX_BUFFER];
    memset(buffer, ' ', MAX_BUFFER);

    View *view = view_create(buffer, size.cols, size.rows);
    View *child = view_from(view, 10, 10, 5, 5);

    for (int i = 0; i < 5; i++) {
        view_write_line(child, STRING("..........", 10), i);
    }

    term_write(view_to_string(view));
    term_set_cursor_pos(0, 0);

    while (true) {
        Input input = term_get_input();
        if (input.control == CONTROL_ENTER)
            break;
    }

    term_restore_buffer();
    return 0;
}
