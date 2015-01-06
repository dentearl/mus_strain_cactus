#include "Math.h"
#include <float.h>

#ifdef __FreeBSD__
#include <ieeefp.h>
#endif

// Double value for positive infinity:  //FIXME: do this right (ieee)
double Math::doubleInf = DBL_MAX;

// Have we initialized the FPU yet?
bool Math::fpuInitialized = false;

// Static instance of class to force initialization.
static Math mathObj;

/*
 * Do system-dependent FPU initialization. 
 */
void Math::fpuInit() {
#ifdef __FreeBSD__
    /*
     * Set mask to generate exceptions of FreeBSD.
     * Omit: FP_X_IMP | FP_X_DNML from the mask.
     */
    (void)fpsetmask(FP_X_INV | FP_X_OFL | FP_X_UFL | FP_X_DZ);
    fpsetround(FP_RN);
    (void)fpsetmask(0L);
#endif

    // Generate infinity.  If this FPEs, something is wrong.  For
    // gcc on the alpha, use -mieee.
    doubleInf *= 2.0;

    fpuInitialized = true;
}

/**
 * Compute an integer power.  Efficient for small powers.
 */
int Math::intPower(int a,
                   int n) {
    int val = 1;
    for (int cnt = 0; cnt < n; cnt++) {
        val *= a;
    }
    return val;
}
    
