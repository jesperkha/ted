#include <string.h>
#include "strbuf.h"
#include "term.h"
#include "util.h"

int main(void)
{
    TermSize size = term_get_size();
    if (size.cols == 0)
        PANIC("failed to get terminal size");

    term_new_buffer();

    #define MAX_BUFFER (1024 * 100)
    byte buffer[MAX_BUFFER];

    usize term_size = size.cols*size.rows;
    memset(buffer, ' ', term_size);
    term_write(STRING(buffer, term_size));

    term_set_cursor_pos(0, 0);
    usize write_count = 0;

    while (true) {
        Input input = term_get_input();
        if (input.control == CONTROL_ENTER)
            break;

        if (input.control == CONTROL_BACKSPACE)
            buffer[write_count--] = ' ';
        else {
            buffer[write_count] = input.character;
            write_count++;
        }
    }

    term_restore_buffer();
    return 0;
}
