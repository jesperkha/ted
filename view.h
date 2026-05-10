#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef uint16_t Color;
typedef unsigned int usize;
typedef unsigned char byte;

/*
 * Input handling
 */

typedef enum ControlKey {
   CONTROL_NONE,
   CONTROL_CTRL,
   CONTROL_ALT,
   CONTROL_ESC,
   CONTROL_ENTER,
   CONTROL_BACKSPACE,
   CONTROL_DELETE,
   CONTROL_TAB,
   CONTROL_LEFT,
   CONTROL_RIGHT,
   CONTROL_UP,
   CONTROL_DOWN,
} ControlKey;

typedef struct Input {
   ControlKey control;
   char character;
} Input;

Input get_terminal_input(void);

/*
 * Strings and size
 */

// String is a string buffer, or slice, which points to some text portion.
// The buffer may be a whole string, or just a view into a longer one.
// For this reason, String should always be passed by value and only freed
// explcitly by its owner.
typedef struct String {
   const char *s;
   usize length;
   bool err;
} String;

#define ERROR_STRING ((String){.s = NULL, .length = 0, .err = true})
#define STRING(ptr, len) ((String){.s = (char *)ptr, .length = len, .err = false})

#define string_println(strbuf) { printf("%.*s", strbuf.length, strbuf.s); }

typedef struct Size {
    usize width;
    usize height;
} Size;

#define SIZE(w, h) ((Size){.width = w, .height = h})

/*
 * View
 */

// A Cell is a single character drawn to the screen. It contains the character
// itself, styling information, and additional flags.
typedef struct Cell {
    Color fg, bg;
    byte flags;
    byte c;
} Cell;

typedef struct View View;

// Create a new root view. Initialize empty Cell buffer.
View *view_create_root(void);

// Free root view. Does nothing if v has a parent.
void view_destroy_root(View *v);

// Get root view of v.
View *view_get_root(View *v);

// Create a view into parent with the given dimensions.
View *view_from(View *parent, usize x, usize y, usize w, usize h);

// Resize the view with delta x,y,w,h. May clamp values based on parent size.
// Returns true on success;
bool view_resize(View *v, int dx, int dy, int dw, int dh);

// Sets the position and and size of the view. Returns true on success.
bool view_set_size(View *v, usize x, usize y, usize w, usize h);

// Get a buffer to render to before writing line to a view. The size of the
// buffer in cells is written to size if not null.
Cell *view_get_line_buffer(usize *size);

// Write count cells to a line y in v at offset x. Will at most write the number
// of cells equal to the view width-x. Returns number of cells written.
usize view_write_line(View *v, Cell *cells, usize x, usize y, usize count);

// Write string at given position with given background and foreground color;
usize view_write_string(View *v, String s, usize x, usize y, Color bg, Color fg);

// Same as view_write_string(), but it fills the remaining width with the given bg color as well.
usize view_write_string_line(View *v, String s, usize x, usize y, Color bg, Color fg);

// Get the cell at x,y. Returns NULL if out of bounds.
Cell *view_cell_at(View *v, usize x, usize y);

// Get the size of v.
Size view_size(View *v);

// Get root view of v and render to the screen. Returns number of bytes written.
usize view_render(View *v);

// Fill the entire view with the given background color.
// Returns number of cells written.
usize view_clear(View *v, Color color);

/*
 * UI Components
 */

// Options for ui popups. A zero option is a valid option and uses default settings.
typedef struct UIOptions {
    // Foreground and background colors. Defaults to white on black if not set.
    Color fg, bg;
    // If the popup should be centered in the view.
    bool centered;
    // If the popup should cover the view.
    bool cover;
    // Minimum width of popup. 0 means none. Only works for centered UIs.
    usize min_width;
    // Number of characters to pad with.
    usize padx, pady;
} UIOptions;

// Create a padded container with a minimum requested width and height.
// Returns a new view inside this container to draw to.
View *ui_container(View *v, usize width, usize height, UIOptions *options);

// Show centered list of items in the given view. Returns -1 if user cancels.
// Otherwise returns index of selected option. Options may be NULL to use defaults.
int ui_option_list(View *v, String prompt, String *items, usize num_items, UIOptions *options);

/*
 * Colors
 */

#define _C(a, b) ((Color)((a) << 8 | (b)))

#define FG_NONE    _C('3', '9')
#define FG_BLACK   _C('3', '0')
#define FG_RED     _C('3', '1')
#define FG_GREEN   _C('3', '2')
#define FG_YELLOW  _C('3', '3')
#define FG_BLUE    _C('3', '4')
#define FG_MAGENTA _C('3', '5')
#define FG_CYAN    _C('3', '6')
#define FG_WHITE   _C('3', '7')

#define BG_NONE    _C('4', '9')
#define BG_BLACK   _C('4', '0')
#define BG_RED     _C('4', '1')
#define BG_GREEN   _C('4', '2')
#define BG_YELLOW  _C('4', '3')
#define BG_BLUE    _C('4', '4')
#define BG_MAGENTA _C('4', '5')
#define BG_CYAN    _C('4', '6')
#define BG_WHITE   _C('4', '7')

#define TO_BG(c) (c + (1 << 8))
#define TO_FG(c) ((c) == BG_NONE ? FG_BLACK : ((c) - (1 << 8)))

/*
 * Library implementation
 */

#ifndef VIEW_IMPLEMENTATION

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>

//              Utils
// -------------------------------------------

#define DEREF_OR(ptr, _default) ((ptr) != NULL ? *(ptr) : (_default))

#define __print_file(prefix, msg) {                                  \
    FILE *f = fopen("log.txt", "a");                                 \
    if (f != NULL) {                                                 \
        fprintf(f, "(%s:%d) "prefix": "msg"\n", __FILE__, __LINE__); \
        fclose(f);                                                   \
    }}

#define __print_filef(prefix, msg, ...) {                                         \
    FILE *f = fopen("log.txt", "a");                                              \
    if (f != NULL) {                                                              \
        fprintf(f, "(%s:%d) "prefix": "msg"\n", __FILE__, __LINE__, __VA_ARGS__); \
        fclose(f);                                                                \
    }}

#define __print_exit(prefix, msg) { __print_file(prefix, msg); exit(1); }

#define TODO(msg) __print_exit("TODO", msg)
#define PANIC(msg) __print_exit("PANIC", msg)

#define LOG(msg) __print_file("LOG", msg)
#define LOGF(msg, ...) __print_filef("LOG", msg, __VA_ARGS__)

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

//              Terminal
// -------------------------------------------

static struct termios orig_termios;
static bool raw_enabled = false;

// Set terminal raw mode according to enable.
static void term_set_raw_mode(bool enable) {
    if (enable && !raw_enabled) {
        tcgetattr(STDIN_FILENO, &orig_termios);

        struct termios raw = orig_termios;

        raw.c_lflag &= ~(ECHO | ICANON | ISIG);
        raw.c_iflag &= ~(IXON | ICRNL);
        raw.c_oflag &= ~(OPOST);

        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        raw_enabled = true;
    } else if (!enable && raw_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_enabled = false;
    }
}

// Get terminal input in raw mode.
Input get_terminal_input(void) {
    Input input = {CONTROL_NONE, 0};
    char c;

    if (read(STDIN_FILENO, &c, 1) != 1) {
        return input;
    }

    if (c == '\x1b') {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        struct timeval tv = {0, 50000}; // 50ms
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) <= 0) {
            input.control = CONTROL_ESC;
            return input;
        }

        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return input;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return input;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': input.control = CONTROL_UP; break;
                case 'B': input.control = CONTROL_DOWN; break;
                case 'C': input.control = CONTROL_RIGHT; break;
                case 'D': input.control = CONTROL_LEFT; break;
            }
        }
        return input;
    }

    switch (c) {
        case '\r':
        case '\n': input.control = CONTROL_ENTER; break;
        case 127: input.control = CONTROL_BACKSPACE; break;
        case '\t': input.control = CONTROL_TAB; break;
        default: {
            if (c >= 1 && c <= 26) {
                input.control = CONTROL_CTRL;
                input.character = c + 'a' - 1;
            } else if (c >= 32 && c <= 126) {
                input.character = c;
            } else {
                input.character = '?';
            }
            break;
        }
    }

    return input;
}

// Set the terminal cursor position.
static void term_set_cursor_pos(usize row, usize col) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\x1b[%u;%uH", row, col);
    write(STDOUT_FILENO, buf, len);
}

// Get the terminal buffer size in rows and columns. Returns zero size on failure.
static Size term_get_size(void) {
    struct winsize ws;
    Size size = {0, 0};

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        return size;
    }

    size.width = ws.ws_col;
    size.height = ws.ws_row;
    return size;
}

// Write buf to the terminal at the cursor position.
static void term_write(String buf) {
    if (buf.err || buf.s == NULL) return;
    write(STDOUT_FILENO, buf.s, buf.length);
}

// Toggle cursor visibility.
static void term_set_cursor_visible(bool visible) {
    if (visible) {
        const char *seq = "\x1b[?25h";
        write(STDOUT_FILENO, seq, 6);
    } else {
        const char *seq = "\x1b[?25l";
        write(STDOUT_FILENO, seq, 6);
    }
}

static bool using_alt_buffer = false;

static void term_enter_alt_buffer(void) {
    if (!using_alt_buffer) {
        write(STDOUT_FILENO, "\x1b[?1049h", 8);
        using_alt_buffer = true;
    }
}

static void term_leave_alt_buffer(void) {
    if (using_alt_buffer) {
        write(STDOUT_FILENO, "\x1b[?1049l", 8);
        using_alt_buffer = false;
    }
}

static void term_signal_handler(int sig) {
    (void)sig;

    term_leave_alt_buffer();
    term_set_cursor_visible(true);
    term_set_raw_mode(false);

    _exit(1);
}

// Restore to previous buffer.
static void term_restore_buffer(void) {
    term_set_cursor_visible(true);
    term_set_raw_mode(false);
    term_leave_alt_buffer();
}

// Create new alternate buffer to draw to and restore to later.
static void term_new_buffer(void) {
    term_enter_alt_buffer();
    term_set_raw_mode(true);

    term_set_cursor_visible(false);

    atexit(term_restore_buffer);

    signal(SIGINT,  term_signal_handler);
    signal(SIGTERM, term_signal_handler);
    signal(SIGQUIT, term_signal_handler);
    signal(SIGHUP,  term_signal_handler);
}

//              View impl
// -------------------------------------------

#define LINE_DRAW_BUFFER_SIZE 512
#define RENDER_BUFFER_SIZE (1024 * 1024)

// Global buffer used to draw a single line to before rendering to a view.
Cell line_draw_buffer[LINE_DRAW_BUFFER_SIZE];

// Internal buffer used to render content to terminal window.
byte render_buffer[RENDER_BUFFER_SIZE];

Cell *view_get_line_buffer(usize *size) {
    if (size != NULL) {
        *size = LINE_DRAW_BUFFER_SIZE;
    }
    return line_draw_buffer;
}

struct View {
    // Parent view if any.
    View *parent;
    // Buffer points to the first cell in the view.
    Cell *buffer;
    usize x, y, width, height;
    // Number of cells from line start to next line start.
    usize line_wrap_amount;
};

View *view_create_root(void) {
    term_new_buffer();

    Size size = term_get_size();
    if (size.width == 0)
        PANIC("failed to get terminal size");

    usize cell_count = size.width * size.height;

    View *v = malloc(sizeof(View));
    v->parent = NULL;
    v->buffer = malloc(sizeof(Cell) * cell_count);
    if (v->buffer == NULL)
        PANIC("failed to allocate view buffer");

    // TODO: view destroy

    // Initialize clear buffer
    for (usize i = 0; i < cell_count; i++) {
        v->buffer[i].bg = BG_NONE;
        v->buffer[i].fg = FG_WHITE;
        v->buffer[i].c = ' ';
    }

    v->x = v->y = 0;
    v->width = size.width;
    v->height = size.height;
    v->line_wrap_amount = size.width;
    return v;
}

static void init_view(View *parent, View *v, usize x, usize y, usize w, usize h) {
    v->parent = parent;
    v->line_wrap_amount = parent->line_wrap_amount;

    usize offset = (y * parent->line_wrap_amount) + x;
    v->buffer = parent->buffer + offset;

    if (x > parent->width || y > parent->height)
        PANIC("position of child view is outside parent");

    // Clamp width and height to be contained in parent.
    if (x + w > parent->width)
        w = parent->width - x;
    if (y + h > parent->height)
        h = parent->height - y;

    v->x = x;
    v->y = y;
    v->width = w;
    v->height = h;
}

View *view_from(View *parent, usize x, usize y, usize w, usize h) {
    if (parent == NULL)
        return NULL;

    View *v = malloc(sizeof(View));
    init_view(parent, v, x, y, w, h);
    return v;
}

usize view_write_line(View *v, Cell *cells, usize x, usize y, usize count) {
    if (v == NULL || cells == NULL || v->buffer == NULL || count == 0)
        return 0;

    if (y >= v->height || x >= v->width)
        return 0;

    usize write_count = min(min(v->width, count), v->width - x);
    Cell *offset = v->buffer + x + (y * v->line_wrap_amount);
    memcpy(offset, cells, write_count * sizeof(Cell));
    return write_count;
}

Size view_size(View *v) {
    if (v == NULL)
        return SIZE(0, 0);
    return SIZE(v->width, v->height);
}

bool view_resize(View *v, int dx, int dy, int dw, int dh) {
    if (v == NULL || v->parent == NULL)
        return false;

    int x = v->x + dx;
    int y = v->y + dy;
    int w = v->width + dw;
    int h = v->height + dh;

    if (x < 0 || y < 0 || w < 0 || h < 0)
        return false;

    init_view(v->parent, v, x, y, w, h);
    return true;
}

bool view_set_size(View *v, usize x, usize y, usize w, usize h) {
    if (v == NULL || v->parent == NULL)
        return false;

    init_view(v->parent, v, x, y, w, h);
    return true;
}

usize view_clear(View *v, uint16_t color) {
    if (v == NULL)
        return 0;

    for (usize i = 0; i < v->height; i++) {
        usize size;
        Cell *cells = view_get_line_buffer(&size);
        usize count = min(size, v->width);

        for (usize i = 0; i < count; i++) {
            cells[i] = (Cell){.c = ' ', .fg = FG_NONE, .bg = color};
        }

        view_write_line(v, cells, 0, i, count);
    }

    return v->width * v->height;
}

Cell *view_cell_at(View *v, usize x, usize y) {
    if (v == NULL || x >= v->width || y >= v->height)
        return NULL;

    return &v->buffer[x + (y * v->line_wrap_amount)];
}

void view_destroy_root(View *v) {
    if (v == NULL || v->parent != NULL)
        return;

    term_restore_buffer();
    free(v->buffer);
    free(v);
}

View *view_get_root(View *v) {
    if (v == NULL)
        return v;

    while (v->parent != NULL)
        v = v->parent;

    return v;
}

usize view_write_string(View *v, String s, usize x, usize y, Color bg, Color fg) {
    Cell *cells = view_get_line_buffer(NULL);
    for (usize i = 0; i < s.length; i++) {
        cells[i] = (Cell){.c = s.s[i], .bg = bg, .fg = fg};
    }

    return view_write_line(v, cells, x, y, s.length);
}

usize view_write_string_line(View *v, String s, usize x, usize y, Color bg, Color fg) {
    Cell *cells = view_get_line_buffer(NULL);
    for (usize i = 0; i < s.length; i++) {
        cells[i] = (Cell){.c = s.s[i], .bg = bg, .fg = fg};
    }
    for (usize i = s.length; i < v->width; i++) {
        cells[i] = (Cell){.c = ' ', .bg = bg, .fg = fg};
    }

    return view_write_line(v, cells, x, y, v->width);
}

static usize write_to_buffer(View *v, byte *buffer, usize max_size) {
    if (v == NULL)
        return 0;

    usize cell_size = 5 + 5 + 1; // Two colors and the character

    // Number of bytes and cells to write
    usize byte_count = min(max_size, v->width * v->height * cell_size);
    usize cell_count = byte_count / cell_size;

    usize n = 0; // Byte position
    for (usize i = 0; i < cell_count; i++) {
        Cell c = v->buffer[i];

        memcpy(buffer+n, "\x1b[", 2);
        n += 2;
        buffer[n++] = c.bg >> 8;
        buffer[n++] = c.bg & 0xff;
        buffer[n++] = 'm';

        memcpy(buffer+n, "\x1b[", 2);
        n += 2;
        buffer[n++] = c.fg >> 8;
        buffer[n++] = c.fg & 0xff;
        buffer[n++] = 'm';

        buffer[n++] = c.c;
    }

    return byte_count;
}

usize view_render(View *v) {
    View *root = view_get_root(v);
    if (root == NULL)
        return 0;

    usize written = write_to_buffer(root, render_buffer, RENDER_BUFFER_SIZE);
    term_write(STRING(render_buffer, written));
    term_set_cursor_pos(0, 0);
    return written;
}

//              UI Components
// -------------------------------------------

static UIOptions *normalize_options(UIOptions *ptr) {
    if (ptr == NULL)
        return ptr;
    if (ptr->bg == 0)
        ptr->bg = BG_BLACK;
    if (ptr->fg == 0)
        ptr->fg = FG_WHITE;
    return ptr;
}

static UIOptions default_popup_options(void) {
    return (UIOptions){
        .bg = BG_BLACK,
        .fg = FG_WHITE,
        .centered = true,
        .cover = false,
        .min_width = 0,
    };
}

// Get normalized options if not null, else default options.
#define get_options(opt) (DEREF_OR(normalize_options(opt), default_popup_options()))

View *ui_container(View *v, usize width, usize height, UIOptions *_options) {
    UIOptions opt = get_options(_options);
    Size size = view_size(v);

    View *container;

    if (opt.cover) {
        container = view_from(v, 0, 0, size.width, size.height);
    } else {
        usize x = max((int)size.width/2 - (int)width/2 - (int)opt.padx, 0);
        usize y = max((int)size.height/2 - (int)height/2 - (int)opt.pady, 0);

        if (opt.min_width != 0)
            width = min(size.width, opt.min_width);

        width = min(width+opt.padx*2, size.width);
        height = min(height+opt.pady*2, size.height);

        container = view_from(v, x, y, width, height);
    }

    view_clear(container, opt.bg);
    view_resize(container, opt.padx, opt.pady, -opt.padx*2, -opt.pady*2);
    return container;
}

int ui_option_list(View *v, String prompt, String *items, usize num_items, UIOptions *_options) {
    UIOptions opt = get_options(_options);

    usize height = num_items + 1 + 1; // 1 blank, 1 prompt

    // Get the longest string in the option list
    usize width = prompt.length;
    for (usize i = 0; i < num_items; i++) {
        width = max(width, items[i].length);
    }

    View *box = ui_container(v, width, height, &opt);
    int selected = 0;

    while (true) {
        view_clear(box, opt.bg);
        view_write_string(box, prompt, 0, 0, opt.bg, opt.fg);

        for (usize i = 0; i < num_items; i++) {
            String option = items[i];

            uint16_t bg = opt.bg;
            uint16_t fg = opt.fg;
            if (i == (usize)selected) {
                bg = TO_BG(opt.fg);
                fg = TO_FG(opt.bg);
            }

            view_write_string_line(box, option, 0, i+2, bg, fg);
        }

        view_render(v);

        Input input = get_terminal_input();
        if (input.control == CONTROL_ESC) {
            selected = -1;
            break;
        }

        if (input.control == CONTROL_DOWN || input.character == 'j')
            selected = min((int)num_items-1, selected+1);
        if (input.control == CONTROL_UP || input.character == 'k')
            selected = max(0, selected-1);
        if (input.control == CONTROL_ENTER || input.character == ' ')
            break;
    }

    free(box);
    return selected;
}

#endif
