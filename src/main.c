#include <string.h>
#include "buffer.h"
#include "term.h"
#include "util.h"
#include "view.h"

int main(void)
{
    Size size = term_get_size();
    if (size.width == 0)
        PANIC("failed to get terminal size");

    term_new_buffer();

    #define MAX_BUFFER (1024 * 100)
    byte buffer[MAX_BUFFER];
    memset(buffer, ' ', MAX_BUFFER);

    View *view = view_create(buffer, size.width, size.height);
    Buffer *b = buffer_create();

    buffer_write(b, STRING("Hello", 5));

    buffer_render(b, view);
    term_write(view_to_string(view));
    term_set_cursor_pos(0, 0);

    while (true) {
        Input input = term_get_input();
        if (input.control == CONTROL_ENTER)
            break;
    }

    buffer_destroy(b);
    term_restore_buffer();
    return 0;
}
