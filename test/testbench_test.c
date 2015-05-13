/*
 * This file is part of Testbench.
 *
 * Copyright 2015 Ironmelt Limited.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.
 */

/**
 * @file
 * @brief Basic testbench playground.
 */

#include "testbench.h"


/******************************************************************************
 * GLOBAL VARIABLES
 */

uint32_t __testbench_test_failed_to_fail = 0;


/******************************************************************************
 * TEST HELPER MACROS
 */

#define MUST_FAIL(__must_fail_block...) \
    { \
      uint32_t __must_fail_diff = __testbench_global_context->total - __testbench_global_context->failed; \
      __must_fail_block; \
      if (__testbench_global_context->total - __testbench_global_context->failed == __must_fail_diff) { \
        __TESTBENCH_PRINT( \
            TESTBENCH_ANSI_COLOR_GREEN "    ✓ This test was supposed to fail and did so" TESTBENCH_ANSI_RESET); \
      } else { \
        __TESTBENCH_PRINT( \
            TESTBENCH_ANSI_COLOR_RED "    ✗ This test was supposed to fail and didn't" TESTBENCH_ANSI_RESET); \
        ++__testbench_test_failed_to_fail; \
      } \
    }


/******************************************************************************
 * TEST FIXTURES
 */


static inline
void * _testbench_fixture_setup(void * udata) {
  if (*((int *) udata) == 42) {
    return (void *) 42;
  }
  return NULL;
}


static inline
void _testbench_fixture_teardown(void * udata, void * fixtures) {
}


/******************************************************************************
 * TESTS
 */


TEST(testbench_assert, {

  DESCRIBE("ASSERT()", {

    IT("P / should assert true correctly", {
      ASSERT(true);
    })

    MUST_FAIL({
      IT("F / should assert false correctly", {
        ASSERT(false);
      })
    })

    MUST_FAIL({
      IT("F / should display output on fail [two messages under this line]", {
        fprintf(stdout, "A message on STDOUT.\n");
        fprintf(stderr, "A message on STDERR.\n");
        FAIL();
      })
    })

  })

})


TEST(testbench_assert_desc, {

  DESCRIBE("ASSERT_DESC()", {

    IT("P / should assert true correctly", {
      ASSERT_DESC(true, "SHOULD NOT DISPLAY");
    })

    MUST_FAIL({
      IT("F / should assert false correctly, and display \"a nice message\"", {
        ASSERT_DESC(false, "a %s message", "nice");
      })
    })

  })

})


TEST(testbench_pass, {

  DESCRIBE("PASS()", {

    IT("P / should pass, and not execute any further instruction", {
      PASS();
      FAIL_DESC("SHOULD NOT DISPLAY");
      ++__testbench_test_failed_to_fail;
    })

  })

})


TEST(testbench_fail, {

  DESCRIBE("FAIL()", {

    MUST_FAIL({
      IT("F / should fail, and not execute any further instruction", {
        FAIL();
        FAIL_DESC("SHOULD NOT DISPLAY");
        ++__testbench_test_failed_to_fail;
      })
    })

  })

})


TEST(testbench_fail_desc, {

  DESCRIBE("FAIL()", {

    MUST_FAIL({
      IT("F / should fail, not execute any further instruction, and display \"a nice message\"", {
        FAIL_DESC("a %s message", "nice");
        FAIL_DESC("SHOULD NOT DISPLAY");
        ++__testbench_test_failed_to_fail;
      })
    })

  })

})


TEST(testbench_run_context, {

  IT("P / should pass user data", {
    ASSERT((intptr_t) udata == 42)
  })

})


TEST(testbench_run, {

  DESCRIBE("RUN()", {
    RUN(testbench_run_context, (void *) 42);
  })

})


TEST(testbench_fixtures, {

  DESCRIBE("fixtures", {

    int * test_udata = malloc(sizeof(intptr_t));
    *test_udata = 42;

    SETUP(_testbench_fixture_setup, test_udata);
    TEARDOWN(_testbench_fixture_teardown, test_udata);

    IT("P / should run setup", {
      ASSERT((intptr_t) fixtures == 42);
    })

    IT("P / should run teardown", {
      PASS(); // TODO: find a nice way to test...
    })

    free(test_udata);

  })

})


/******************************************************************************
 * TEST RUNNER
 */


int main() {

  DESCRIBE("Testbench", {

    RUN(testbench_assert, NULL);
    RUN(testbench_assert_desc, NULL);
    RUN(testbench_pass, NULL);
    RUN(testbench_fail, NULL);
    RUN(testbench_fail_desc, NULL);
    RUN(testbench_run, NULL);
    RUN(testbench_fixtures, NULL);

  })

  RESULTS();

  if (!__testbench_test_failed_to_fail) {
    fprintf(
        stderr,
        TESTBENCH_ANSI_BOLD TESTBENCH_ANSI_COLOR_GREEN
        "✓ All tests expected to fail have failed.\n\n" TESTBENCH_ANSI_RESET);
  } else {
    fprintf(
        stderr,
        TESTBENCH_ANSI_BOLD TESTBENCH_ANSI_COLOR_RED
        "✗ Some tests expected to fail didn't.\n\n" TESTBENCH_ANSI_RESET);
  }

  return (int) !!__testbench_test_failed_to_fail;
}
