// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#ifndef OMNIC_MACROS_H_
#define OMNIC_MACROS_H_

/* -------------------------------------------------------------------------- */

/// @file macros.h
/// @brief Header for macro utilities.

/* -------------------------------------------------------------------------- */

// Assertion macro for general conditions
#define ASSERT(condition, message)                                   \
  do {                                                               \
    if (!(condition)) {                                              \
      (void)fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, \
                    (message));                                      \
      g_test_failures++;                                             \
    }                                                                \
  } while ((void)0, 0)

// Assertion macro for comparing values (provides more context on failure)
#define ASSERT_EQ(a, b, fmt, message)                                       \
  do {                                                                      \
    if ((a) != (b)) {                                                       \
      (void)fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__,        \
                    (message));                                             \
      (void)fprintf(stderr, "      Expected: " fmt ", Got: " fmt "\n", (b), \
                    (a));                                                   \
      g_test_failures++;                                                    \
    }                                                                       \
  } while ((void)0, 0)

#endif  // OMNIC_MACROS_H_
