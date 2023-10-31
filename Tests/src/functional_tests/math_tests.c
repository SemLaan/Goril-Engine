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

static bool inverse_matrix_test()
{
    mat4 mat = mat4_rotate_x(3);
    mat = mat4_mul_mat4(mat, mat4_scale((vec3){100, 32, -33}));

    mat4 inverse = mat4_inverse(mat);

    mat4 result = mat4_mul_mat4(mat, inverse);
    mat4 identity = mat4_identity();

    for (u32 i = 0; i < 16/*elements of 4x4 mat*/; ++i)
	{
        expect_float_to_be(identity.values[i], result.values[i]);
	}

    return true;
}


void register_math_tests()
{
    register_test(determinant_test, "Math: Determinants");
    register_test(inverse_matrix_test, "Math: Inverse matrix");
}