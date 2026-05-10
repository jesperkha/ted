#include "view.h"

int main(void) {
    View *root = view_create_root();

    UIOptions options = {
        .bg = BG_MAGENTA,
        .padx = 2,
        .pady = 1,
        .cover = true,
    };

    View *container = ui_container(root, 0, 0, &options);

    String items[3] = {
        STRING("Hello", 5),
        STRING("There", 5),
        STRING("Bob", 3),
    };

    options = (UIOptions){
        .bg = BG_BLUE,
        .padx = 2,
        .pady = 1,
    };

    ui_option_list(container, STRING("Which option?", 13), items, 3, &options);

    view_destroy_root(root);
    return 0;
}
