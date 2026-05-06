#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef unsigned int usize;
typedef unsigned char byte;

#define __print_exit(prefix, msg) { printf("(%s:%d) "prefix": %s\n", __FILE__, __LINE__, msg); exit(1); }

#define TODO(msg) __print_exit("TODO", msg)
#define PANIC(msg) __print_exit("PANIC", msg)

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)

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
