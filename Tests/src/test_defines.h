#pragma once
#include <core/logger.h>
#include <math.h>

// Extra log define that always gets ran regardless of debug/release/dist because that doesn't matter for tests
// Yet we don't want to have to log errors or fatals because thats not what were trying to log
#define TESTINFO(message, ...)	::GR::Log(::GR::LOG_LEVEL_INFO, std::format(message, __VA_ARGS__))



#define expect_should_be(expected, actual)                                                              \
    if (actual != expected) {                                                                           \
        GRERROR("--> Expected {}, but got: {}. File: {}:{}.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                                   \
    }

#define expect_should_not_be(expected, actual)                                                                   \
    if (actual == expected) {                                                                                    \
        GRERROR("--> Expected {} != {}, but they are equal. File: {}:{}.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                                            \
    }

#define expect_float_to_be(expected, actual)                                                        \
    if (std::abs(expected - actual) > 0.001f) {                                                         \
        GRERROR("--> Expected {}, but got: {}. File: {}:{}.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                               \
    }

#define expect_to_be_true(actual)                                                      \
    if (actual != true) {                                                              \
        GRERROR("--> Expected true, but got: false. File: {}:{}.", __FILE__, __LINE__); \
        return false;                                                                  \
    }

#define expect_to_be_false(actual)                                                     \
    if (actual != false) {                                                             \
        GRERROR("--> Expected false, but got: true. File: {}:{}.", __FILE__, __LINE__); \
        return false;                                                                  \
    }