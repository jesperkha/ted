#include "ui.h"
#include "util.h"
#include "view.h"

int main(void) {
    View *root = view_create_root();

    String options[3] = {
        STRING("Hello", 5),
        STRING("There", 5),
        STRING("Bob", 3),
    };

    int selected = ui_option_list(root, STRING("Which option?", 13), options, 3);

    view_destroy_root(root);
    return 0;
}
