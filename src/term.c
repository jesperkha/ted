#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

#include "term.h"

static struct termios orig_termios;
static bool raw_enabled = false;

void term_set_raw_mode(bool enable) {
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

Input term_get_input(void) {
    Input input = {CONTROL_NONE, 0};
    char c;

    if (read(STDIN_FILENO, &c, 1) != 1) {
        return input;
    }

    if (c == '\x1b') {
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
        default:
            if (c >= 1 && c <= 26) {
                input.control = CONTROL_CTRL;
                input.character = c + 'a' - 1;
            } else {
                input.character = c;
            }
            break;
    }

    return input;
}

void term_set_cursor_pos(usize row, usize col) {
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "\x1b[%u;%uH", row, col);
    write(STDOUT_FILENO, buf, len);
}

Size term_get_size(void) {
    struct winsize ws;
    Size size = {0, 0};

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        return size;
    }

    size.width = ws.ws_col;
    size.height = ws.ws_row;
    return size;
}

void term_write(String buf) {
    if (buf.err || buf.s == NULL) return;
    write(STDOUT_FILENO, buf.s, buf.length);
}

void term_clear(void) {
    const char *seq = "\x1b[2J\x1b[H";
    write(STDOUT_FILENO, seq, 7);
}

void term_set_cursor_visible(bool visible) {
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

void term_new_buffer(void) {
    term_enter_alt_buffer();

    term_set_cursor_visible(false);

    atexit(term_restore_buffer);

    signal(SIGINT,  term_signal_handler);
    signal(SIGTERM, term_signal_handler);
    signal(SIGQUIT, term_signal_handler);
    signal(SIGHUP,  term_signal_handler);
}

void term_restore_buffer(void) {
    term_set_cursor_visible(true);
    term_set_raw_mode(false);
    term_leave_alt_buffer();
}
