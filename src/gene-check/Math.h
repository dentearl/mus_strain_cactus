#ifndef MATH_MATH_H
#define MATH_MATH_H

#include "Types.h"
#include "Real.h"
#include <math.h>

// Get rid of macros that cause confusing errors
#undef min
#undef max

/**
 * Various math operations.
 */
class Math {
 private:
    //FIXME: convert to using IEEE.
    static double doubleInf;  // Double value for positive infinity.

    static bool fpuInitialized;  // Have we initialized the FPU yet?

    /**
     * Initialize the FPU flags.
     */
    void fpuInit();

  public:
    /**
     * Constructor.  Math.cc creates a single instance to trigger setting of
     * FPU flags.
     */
    Math() {
        if (!fpuInitialized) {
            fpuInit();
        }
    }

    /*
     * Absolute value of a real number.
     */
    static inline Real abs(Real n) {
        return (n >= 0.0) ? n : -n;
    }

    /*
     * Max of two real numbers.
     */
    static inline Real max(Real n1,
                           Real n2) {
        return (n1 > n2) ? n1 : n2;
    }

    /*
     * Min of two real numbers.
     */
    static inline Real min(Real n1,
                           Real n2) {
        return (n1 < n2) ? n1 : n2;
    }

    /*
     * Max of two integers.
     */
    static inline int max(int n1,
                          int n2) {
        return (n1 > n2) ? n1 : n2;
    }

    /*
     * Min of two integers.
     */
    static inline int min(int n1,
                          int n2) {
        return (n1 < n2) ? n1 : n2;
    }

    /*
     * Max of two unsigneds.
     */
    static inline unsigned max(unsigned n1,
                               unsigned n2) {
        return (n1 > n2) ? n1 : n2;
    }

    /*
     * Min of two unsigneds.
     */
    static inline unsigned min(unsigned n1,
                               unsigned n2) {
        return (n1 < n2) ? n1 : n2;
    }

    /**
     * Compare two integers in the manner of strcmp.
     */
    static inline int cmp(int n1,
                          int n2) {
        if (n1 < n2) {
            return -1;
        } else if (n1 > n2) {
            return 1;
        } else {
            return 0;
        }
    }

    /**
     * Compare two doubles in the manner of strcmp.
     */
    static inline int cmp(double n1,
                          double n2) {
        if (n1 < n2) {
            return -1;
        } else if (n1 > n2) {
            return 1;
        } else {
            return 0;
        }
    }

    /**
     * Compute the squared Euclidean distance between two points using:
     *
     *     d^2(x,y) = K(x,x) - 2 K(x,y) + K(y,y)
     */
    static Real computeSquaredDistance(Real kxx,
                                       Real kxy,
                                       Real kyy) {
        return (kxx - (2 * kxy) + kyy);
    }

    /**
     * Get the value of infinity for a double.
     */
    static double getDoubleInf() {
        return doubleInf;
    }

    /**
     * Compute an integer power.  Efficient for small powers.
     */
    static int intPower(int a,
                        int n);
    
};
#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
