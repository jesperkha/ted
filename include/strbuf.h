#pragma once

#include <stdbool.h>
#include <stdio.h>

// StrBuf is a string buffer, or slice, which points to some text portion.
// The buffer may be a whole string, or just a view into a longer one.
// For this reason, StrBuf should always be passed by value and only freed
// explcitly by its owner.
typedef struct String {
   const char *s;
   unsigned int length;
   bool err;
} String;

#define ERROR_STRING ((String){.s = NULL, .length = 0, .err = true})
#define STRING(ptr, len) ((String){.s = ptr, .length = len, .err = false})

#define string_println(strbuf) { printf("%.*s", strbuf.length, strbuf.s); }
