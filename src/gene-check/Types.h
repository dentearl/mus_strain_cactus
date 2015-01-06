#ifndef SHARED_TYPES_TYPES_H
#define SHARED_TYPES_TYPES_H

#include <assert.h>
#include <typeinfo>

/**
 * @name instanceOf
 *
 * Check at run time if an object is an instance of a class.  This differs
 * from comparing the result of type_id, as it takes inheritance into account.
 *
 * @param objPtr A pointer to the object to check.  A pointer must be used, not
 *  an object.  If necessary, generate an address.
 * @param The class name to test against.
 * @return true if the object is an instance of the class.
 */
#define instanceOf(objPtr, className) \
    ((bool)(dynamic_cast<const className*>(objPtr) != NULL))

#endif
