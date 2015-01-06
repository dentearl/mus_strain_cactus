#ifndef TYPES_REAL_H
#define TYPES_REAL_H

#include <float.h>

// FIXME: this global, Real type din't really work out, need to control
// this individually (Doh!)

/**
 * Real number type (could be double or float), with min and max (*positive*)
 * values.
 */
#if 1
/* as double */
typedef double Real;
#define MAX_REAL DBL_MAX
#define MIN_REAL DBL_MIN
#else
/* as float */
typedef float Real;
#define MAX_REAL FLT_MAX
#define MIN_REAL FLT_MIN
#endif

#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
