#ifndef _MODDEFS_H_
#define _MODDEFS_H_

#include <limits.h>


// Error codes
#define MOD_ERR_ARG             -1
#define MOD_ERR_RESOURCE        -2
#define MOD_ERR_STATE           -3
#define MOD_ERR_BAD_AMD         -4
#define MOD_ERR_BAD_OVERRUN     -5
#define MOD_ERR_BAD_INSTANCE    -6

// Get a size of a array
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

// Increment integet while climping it within int 16 boundaries
#define INC_SAT_U16(a) do {(a) += ((a) == UINT16_MAX ? 0 : 1); } while(0)

// Clamp a numeric value between a lower and upper limit, inclusive.
#define CLAMP(a, low, high) ((a) <= (low) ? ((a) > (high) ? (high) : (a)))

#endif