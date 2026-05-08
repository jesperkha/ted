#pragma once

#include <stdint.h>

#define _C(a, b) ((uint16_t)((a) << 8 | (b)))

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
