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

    IT("F / should assert false correctly", {
      ASSERT(false);
    })

    IT("F / should display output on fail [two messages under this line]", {
      fprintf(stdout, "A message on STDOUT.\n");
      fprintf(stderr, "A message on STDERR.\n");
      FAIL();
    })

  })

})


TEST(testbench_assert_desc, {

  DESCRIBE("ASSERT_DESC()", {

    IT("P / should assert true correctly", {
      ASSERT_DESC(true, "SHOULD NOT DISPLAY");
    })

    IT("F / should assert false correctly, and display \"a nice message\"", {
      ASSERT_DESC(false, "a %s message", "nice");
    })

  })

})


TEST(testbench_pass, {

  DESCRIBE("PASS()", {

    IT("P / should pass, and not execute any further instruction", {
      PASS();
      FAIL_DESC("SHOULD NOT DISPLAY");
    })

  })

})


TEST(testbench_fail, {

  DESCRIBE("FAIL()", {

    IT("F / should fail, and not execute any further instruction", {
      FAIL();
      FAIL_DESC("SHOULD NOT DISPLAY");
    })

  })

})


TEST(testbench_fail_desc, {

  DESCRIBE("FAIL()", {

    IT("F / should fail, not execute any further instruction, and display \"a nice message\"", {
      FAIL_DESC("a %s message", "nice");
      FAIL_DESC("SHOULD NOT DISPLAY");
    })

  })

})


TEST(testbench_run_context, {

  IT("P / should pass user data", {
    ASSERT((int) udata == 42)
  })

})


TEST(testbench_run, {

  DESCRIBE("RUN()", {
    RUN(testbench_run_context, (void *) 42);
  })

})


TEST(testbench_fixtures, {

  DESCRIBE("fixtures", {

    int * test_udata = malloc(sizeof(int));
    *test_udata = 42;

    SETUP(_testbench_fixture_setup, test_udata);
    TEARDOWN(_testbench_fixture_teardown, test_udata);

    IT("P / should run setup", {
      ASSERT((int) fixtures == 42);
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

  return 0;
}
