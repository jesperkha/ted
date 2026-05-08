#include "ui.h"
#include "color.h"
#include "util.h"
#include "view.h"
#include "term.h"
#include <stdint.h>

int ui_option_list(View *v, String prompt, String *options, usize num_options) {
    Size size = view_size(v);

    // Get the longest string in the option list
    usize width = prompt.length;
    for (usize i = 0; i < num_options; i++) {
        width = max(width, options[i].length);
    }

    usize height = num_options + 1 + 1; // 1 blank, 1 prompt
    usize x = (size.width / 2) - (width / 2);
    usize y = (size.height / 2) - (height / 2);

    View *child = view_from(v, x, y, width, height);
    int selected = 0;

    while (true) {
        view_clear(child, BG_BLUE);

        view_write_string(child, prompt, 0, 0, BG_BLUE, FG_WHITE);

        for (usize i = 0; i < num_options; i++) {
            String option = options[i];

            uint16_t bg = BG_BLUE;
            uint16_t fg = FG_WHITE;
            if (i == (usize)selected) {
                bg = BG_WHITE;
                fg = FG_BLUE;
            }

            view_write_string(child, option, 0, i+2, bg, fg);
        }

        view_render(v);

        Input input = term_get_input();
        if (input.control == CONTROL_ESC) {
            selected = -1;
            break;
        }
    }

    free(child);
    return selected;
}
