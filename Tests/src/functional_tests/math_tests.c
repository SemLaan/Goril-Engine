#include "test_register_functions.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <core/meminc.h>
#include <math/lin_alg.h>


static bool determinant_test()
{
    // Mat 2 tests
    {
        mat2 mat = mat2_identity();

        expect_float_to_be(1.f, mat2_determinant(mat));

        mat.values[M2a] = 3;
        mat.values[M2b] = 6;
        mat.values[M2c] = 74;
        mat.values[M2d] = 5;

        expect_float_to_be(-429.f, mat2_determinant(mat));
    }

    // Mat 3 tests
    {
        mat3 mat = mat3_identity();

        expect_float_to_be(1.f, mat3_determinant(mat));
    	
        mat.values[M3b] = 7;
        mat.values[M3c] = 3;
        mat.values[M3d] = 4;
        mat.values[M3f] = 5;
        mat.values[M3g] = 6;
        mat.values[M3h] = 4;

        expect_float_to_be(193.f, mat3_determinant(mat));
    }

    return true;
}


void register_math_tests()
{
    register_test(determinant_test, "Math: Determinants");
}