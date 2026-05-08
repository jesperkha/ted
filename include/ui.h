#pragma once
#include "util.h"
#include "view.h"

// Show centered list of options in the given view. Returns -1 if user cancels.
// Otherwise returns index of selected option.
int ui_option_list(View *v, String prompt, String *options, usize num_options);
