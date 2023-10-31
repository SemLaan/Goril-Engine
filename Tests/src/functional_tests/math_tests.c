#include "test_register_functions.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <core/meminc.h>


static bool determinant_test()
{
    return true;
}


void register_math_tests()
{
    register_test(determinant_test, "Math: Determinants");
}