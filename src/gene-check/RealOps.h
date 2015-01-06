
#ifndef REAL_OPS_H
#define REAL_OPS_H

#include "Real.h"
#include "FatalError.h"

/**
 * Operations on Reals and arrays of reals.
 */
class RealOps {
  private:
    /*
     * Callback for qsort to compare to two real number and sort in assending
     * order.
     */
    static int compare(const void* num1Ptr, const void* num2Ptr) {
        Real num1 = *((Real*)num1Ptr);
        Real num2 = *((Real*)num2Ptr);
        if (num1 > num2) {
            return 1;
        } else if (num1 < num2) {
            return -1;
        } else {
            return 0;
        }
    }

  public:
    /**
     * Sort an array of reals into assending order.
     */
    static void sortArray(Real* realArray,
                          int size) {
        qsort(realArray, size, sizeof(Real), compare);
    }

    /**
     * Get the median in an array of reals.  If there are an
     * even number of elements, average the two median values.
     * Has the side-affect of sorting the array.
     */
    static Real findMedian(Real* realArray,
                           int size) {
        sortArray(realArray, size);

        int midPoint = size / 2;
        if ((size & 1) != 0) {
            return realArray[midPoint];
        } else {
            return (realArray[midPoint-1] + realArray[midPoint]) / 2.0;
        }
    }

    /**
     * Convert a string to a Real.
     */
    static Real toReal(const string& realStr) {
        const char* cstr = realStr.c_str();
        char *endPtr;
        double value = strtod(cstr, &endPtr);

        // Check for trailing garbage, but skip write space.
        // However, if the pointer didn't move, we don't have
        // a number.
        if (endPtr != cstr) {
            while ((*endPtr != '\0') && isspace(*endPtr)) {
                endPtr++;
            }
        }
        if ((endPtr == cstr) || (*endPtr != '\0')) {
            throw FatalError("Invalid real number: \"" + realStr + "\"");
        }
        return (Real)value;
    }
};
#endif

/*
 * Local Variables:
 * mode: c++
 * End:
 */
