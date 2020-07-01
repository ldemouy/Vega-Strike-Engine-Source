#ifndef _ALPHACURVE_H_
#define _ALPHACURVE_H_

#include <math.h>

// query = requested pixel alpha					any
// maxrez x = width of the image in pixels				any
// min = minimum alpha						min 0
// max = maximum alpha						max 255
// focus = multiplier of focus center for curve			0-1
// concavity = multiplier from linear -> terminal(0 or 255)	0-1
// tail_mode_start = overriding slope value for start		negative = standard, 0=flat, high=vertical
// tail_mode_end = overriding slope value for start		negative = standard, 0=flat, high=vertical

int32_t get_alpha(int32_t query, int32_t maxrez_x, int32_t min, int32_t max, double focus, double concavity,
                  int32_t tail_mode_start, int32_t tail_mode_end);

#endif
