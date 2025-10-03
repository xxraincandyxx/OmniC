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
