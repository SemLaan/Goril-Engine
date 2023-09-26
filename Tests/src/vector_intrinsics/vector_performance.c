#include "vector_performance.h"

#include "test_defines.h"
#include "../test_manager.h"
#include <stdlib.h>
#include <immintrin.h>

#define FLOAT_COUNT_ 500001
#define TEST_RUN_COUNT 1000
static float *a;
static float *b;
static float *c;
static float *correct_c;

static bool non_vectorized_float_calculations()
{
    for (int testRuns = 0; testRuns < TEST_RUN_COUNT; ++testRuns)
    {
        for (int i = 0; i < FLOAT_COUNT_; ++i)
        {
            c[i] = a[i] * b[i];
        }
    }

    return true;
}

static bool vectorized_float_calculations_128()
{
    const int vectorCount = FLOAT_COUNT_ / 4;
    const int leftover = FLOAT_COUNT_ % 4;

    for (int testRuns = 0; testRuns < TEST_RUN_COUNT; ++testRuns)
    {
        __m128 a_vec, b_vec, c_vec;

        float *tempA = a;
        float *tempB = b;
        float *tempC = c;

        for (int i = 0; i < vectorCount; ++i)
        {
            a_vec = _mm_loadu_ps(tempA);
            b_vec = _mm_loadu_ps(tempB);

            c_vec = _mm_mul_ps(a_vec, b_vec);

            _mm_storeu_ps(tempC, c_vec);

            tempA = tempA + 4;
            tempB = tempB + 4;
            tempC = tempC + 4;
        }

        for (int i = 0; i < leftover; ++i)
        {
            tempC[i] = tempA[i] * tempB[i];
        }
    }

    return true;
}

static bool vectorized_float_calculations_256()
{
    const int vectorCount = FLOAT_COUNT_ / 8;
    const int leftover = FLOAT_COUNT_ % 8;

    for (int testRuns = 0; testRuns < TEST_RUN_COUNT; ++testRuns)
    {
        __m256 a_vec, b_vec, c_vec;

        float *tempA = a;
        float *tempB = b;
        float *tempC = c;

        for (int i = 0; i < vectorCount; ++i)
        {
            a_vec = _mm256_loadu_ps(tempA);
            b_vec = _mm256_loadu_ps(tempB);

            c_vec = _mm256_mul_ps(a_vec, b_vec);

            _mm256_storeu_ps(tempC, c_vec);

            tempA = tempA + 8;
            tempB = tempB + 8;
            tempC = tempC + 8;
        }

        for (int i = 0; i < leftover; ++i)
        {
            tempC[i] = tempA[i] * tempB[i];
        }
    }

    return true;
}

static bool vectorized_float_calculations_512()
{
    const int vectorCount = FLOAT_COUNT_ / 16;
    const int leftover = FLOAT_COUNT_ % 16;

    for (int testRuns = 0; testRuns < TEST_RUN_COUNT; ++testRuns)
    {
        __m512 a_vec, b_vec, c_vec;

        float *tempA = a;
        float *tempB = b;
        float *tempC = c;

        for (int i = 0; i < vectorCount; ++i)
        {
            a_vec = _mm512_loadu_ps(tempA);
            b_vec = _mm512_loadu_ps(tempB);

            c_vec = _mm512_mul_ps(a_vec, b_vec);

            _mm512_storeu_ps(tempC, c_vec);

            tempA = tempA + 16;
            tempB = tempB + 16;
            tempC = tempC + 16;
        }

        for (int i = 0; i < leftover; ++i)
        {
            tempC[i] = tempA[i] * tempB[i];
        }
    }

    return true;
}

static bool vectorized_float_calculations_512_aligned()
{
    const int vectorCount = FLOAT_COUNT_ / 16;
    const int leftover = FLOAT_COUNT_ % 16;

    for (int testRuns = 0; testRuns < TEST_RUN_COUNT; ++testRuns)
    {
        __m512 a_vec, b_vec, c_vec;

        float *tempA = a;
        float *tempB = b;
        float *tempC = c;

        for (int i = 0; i < vectorCount; ++i)
        {
            a_vec = _mm512_load_ps(tempA);
            b_vec = _mm512_load_ps(tempB);

            c_vec = _mm512_mul_ps(a_vec, b_vec);

            _mm512_store_ps(tempC, c_vec);

            tempA = tempA + 16;
            tempB = tempB + 16;
            tempC = tempC + 16;
        }

        for (int i = 0; i < leftover; ++i)
        {
            tempC[i] = tempA[i] * tempB[i];
        }
    }

    return true;
}

static bool float_calculations_16()
{
    for (int testRuns = 0; testRuns < TEST_RUN_COUNT * 10000; ++testRuns)
    {
        for (int i = 0; i < 16; ++i)
        {
            c[i] = a[i] * b[i];
        }
    }

    return true;
}

static bool vectorized_float_calculations_16()
{
    __m512 a_vec, b_vec, c_vec;

    a_vec = _mm512_load_ps(a);
    b_vec = _mm512_load_ps(b);

    for (int testRuns = 0; testRuns < TEST_RUN_COUNT * 10000; ++testRuns)
    {

        c_vec = _mm512_mul_ps(a_vec, b_vec);
    }

    _mm512_store_ps(c, c_vec);

    return true;
}

static bool check_results_previous()
{
    for (int i = 0; i < FLOAT_COUNT_; ++i)
    {
        if (correct_c[i] != c[i])
            return false;
    }

    return true;
}

void register_vector_perf_tests()
{
    a = _aligned_malloc((FLOAT_COUNT_ + 1) * sizeof(float), 64);
    b = _aligned_malloc((FLOAT_COUNT_ + 1) * sizeof(float), 64);
    c = _aligned_malloc((FLOAT_COUNT_ + 1) * sizeof(float), 64);
    correct_c = _aligned_malloc((FLOAT_COUNT_ + 1) * sizeof(float), 64);

    a += 1;
    b += 1;
    c += 1;
    correct_c += 1;

    for (int i = 0; i < FLOAT_COUNT_; ++i)
    {
        a[i] = i << 3 * 2;
        b[i] = i * 5 ^ i;
    }

    for (int i = 0; i < FLOAT_COUNT_; ++i)
    {
        correct_c[i] = a[i] * b[i];
    }

    register_test(non_vectorized_float_calculations, "non vectorized float calculations");
    register_test(check_results_previous, "Checking results of previous test");
    register_test(vectorized_float_calculations_128, "vectorized float calculations 128");
    register_test(check_results_previous, "Checking results of previous test");
    register_test(vectorized_float_calculations_256, "vectorized float calculations 256");
    register_test(check_results_previous, "Checking results of previous test");
    register_test(vectorized_float_calculations_512, "vectorized float calculations 512");
    register_test(check_results_previous, "Checking results of previous test");
    a -= 1;
    b -= 1;
    c -= 1;
    correct_c -= 1;
    register_test(vectorized_float_calculations_512_aligned, "vectorized float calculations 512 aligned");
    register_test(float_calculations_16, "float calcs 16");
    register_test(vectorized_float_calculations_16, "vectorized float calcs 16");
    // register_test(check_results_previous, "Checking results of previous test");
}