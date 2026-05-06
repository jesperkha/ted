#pragma once

#include <stdlib.h>
#include <stdio.h>

typedef unsigned int usize;
typedef unsigned char byte;

#define __print_exit(prefix, msg) { printf("(%s:%d) "prefix": %s\n", __FILE__, __LINE__, msg); exit(1); }

#define TODO(msg) __print_exit("TODO", msg)
#define PANIC(msg) __print_exit("PANIC", msg)
