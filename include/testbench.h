/*
 * This file is part of Testbench.
 *
 * Copyright 2015 Ironmelt Limited.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, version 3 (AGPLv3).
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * version 3along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/**
 * @file
 * @brief Ironmelt Testbench headers.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


/*****************************************************************************/


#include <fcntl.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


/******************************************************************************
 * CONSTANTS DEFINITION
 */

#define TESTBENCH_ERROR_STRING_MAX_LEN  255
#define TESTBENCH_ERROR_BUF_LEN         255


/******************************************************************************
 * ANSI COLORS
 */

#define TESTBENCH_ANSI_RESET            "\x1b[0m"
#define TESTBENCH_ANSI_BOLD             "\x1b[1m"
#define TESTBENCH_ANSI_COLOR_RED        "\x1b[31m"
#define TESTBENCH_ANSI_COLOR_GREEN      "\x1b[32m"
#define TESTBENCH_ANSI_COLOR_LIGHT_GRAY "\x1b[37m"
#define TESTBENCH_ANSI_COLOR_DARK_GRAY  "\x1b[90m"


/******************************************************************************
 * TYPES
 */

/**
 * Indicates the type of block currently on top of the stack.
 */
typedef enum testbench_block_type_e {

  /**
   * If currently at the root level.
   */
  BLOCK_ROOT,

  /**
   * If currently in a DESCRIBE block.
   */
  BLOCK_DESCRIBE,

  /**
   * If currently in a IT block.
   */
  BLOCK_IT

} testbench_block_type_t;

/**
 * The per-block context of the test.
 */
typedef struct testbench_block_context_s {

  /**
   * The block type.
   */
  testbench_block_type_t block_type;

  /**
   * The block level.
   */
  uint32_t level;

  /**
   * Setup function.
   */
  void * (* setup)(void * udata);

  /**
   * User context to pass to the setup function, if any.
   */
  void * setup_udata;

  /**
   * Teardown function.
   */
  void (* teardown)(void * udata, void * fixtures);

  /**
   * User context to pass to the teardown function, if any.
   */
  void * teardown_udata;

} testbench_block_context_t;

/**
 * The global context of the test.
 */
typedef struct testbench_global_context_s {

  /**
   * Total of failed tests.
   */
  uint32_t failed;

  /**
   * Total number of tests run.
   */
  uint32_t total;

  /**
   * Current block context.
   */
  testbench_block_context_t * block_context;

} testbench_global_context_t;


/******************************************************************************
 * STATIC VARIABLES
 */

/**
 * The root context.
 */
static
testbench_block_context_t __testbench_root_context = {
  .block_type       = BLOCK_ROOT,
  .level            = 0,
  .setup            = NULL,
  .setup_udata    = NULL,
  .teardown         = NULL,
  .teardown_udata = NULL
};

/**
 * Stack-allocated global context.
 */
static
testbench_global_context_t __testbench_global_context_tmp = {
  .total =  0,
  .failed = 0,
  .block_context = &__testbench_root_context
};

/**
 * The global context, initialized to the initial global context we just defined.
 */
static
testbench_global_context_t * __testbench_global_context = &__testbench_global_context_tmp;


/******************************************************************************
 * STATIC FUNCTIONS
 */

/**
 * Print an error message, observing the correct indentation level.
 */
static
void _testbench_print_error(int stream, uint32_t level) {
  bool written = false;
  char buf;
  while (read(stream, &buf, 1) > 0) {
    if (!written) {
      fprintf(stderr, "\n");
      for (int d = 0; d < level; ++d) {
        fprintf(stderr, "  ");
      }
    }
    fprintf(stderr, "%c", buf);
    if (buf == '\n') {
      for (int d = 0; d < level; ++d) {
        fprintf(stderr, "  ");
      }
    }
    written = true;
  }
  if (written) {
    fprintf(stderr, "\n");
  }
}


/******************************************************************************
 * HELPER MACROS
 */

/**
 * Print a formatted line, observing the current context indentation level.
 */
#define __TESTBENCH_PRINT(__format, ...) \
  ({ \
    uint32_t level = __testbench_global_context->block_context->level; \
    if (level == 1) { \
      fprintf(stderr, "\n" TESTBENCH_ANSI_BOLD); \
    } \
    for (int d = 0; d < level - 1; ++d) { \
      fprintf(stderr, "  "); \
    } \
    fprintf(stderr, __format TESTBENCH_ANSI_RESET "\n", ## __VA_ARGS__); \
  })


/******************************************************************************
 * CONTROL MACROS
 */

/**
 * Independent test function.
 */
#define TEST(__name, __test_block) \
  static void _testbench_block_ ## __name ( \
      testbench_global_context_t * __testbench_global_context, \
      void * udata) { \
    __test_block; \
  }

/**
 * Run a test function inside the current test context.
 */
#define RUN(__name, __udata) \
  (_testbench_block_ ## __name (__testbench_global_context, (__udata)))

/**
 * Enter new block.
 */
#define DESCRIBE(__name, __describe_block) \
  { \
    testbench_block_context_t * __testbench_parent_context = \
        __testbench_global_context->block_context; \
    testbench_block_context_t __testbench_local_context = { \
      .block_type       = BLOCK_DESCRIBE, \
      .level            = __testbench_parent_context->level + 1, \
      .setup            = __testbench_parent_context->setup, \
      .setup_udata    = __testbench_parent_context->setup_udata, \
      .teardown         = __testbench_parent_context->teardown, \
      .teardown_udata = __testbench_parent_context->teardown_udata \
    }; \
    __testbench_global_context->block_context = &__testbench_local_context; \
    __TESTBENCH_PRINT(__name); \
    __describe_block; \
    __testbench_global_context->block_context = __testbench_parent_context; \
  }

/**
 * Actual test context.
 *
 * A little explanations about what's going on here:
 *
 * First, a new block context is created, increasing the indentation level, and inheriting from
 * setup and teardown configuration. Two pipes are then created, one for the control, and one for
 * the actual outputs of the test. After this is done, we do a fork of the program.
 *
 * The child, actually running the tests, has his stderr and studout redirected to the pipe we
 * created before. The setup function, if any, is called. Then, a jump point is set. This is where
 * we will jump back in case of a failure of the current test. Then, the actual code of the tests
 * are ran, and the teardown function, if any, is called.
 *
 * The parent process waits for the thread to finish. If it terminates with a value of 0, the test
 * is successful, so we can log it, and discard the pipe. If not, we will actually print the outputs
 * of the test to the console from the pipe, and increment the number of failed tests.
 *
 * Before returning, the previous context is restored, and the total number of executes tests is
 * incremented.
 */
#define IT(__name, __it_block) \
  { \
    testbench_block_context_t * __testbench_parent_context = \
        __testbench_global_context->block_context; \
    testbench_block_context_t __testbench_local_context = { \
      .block_type       = BLOCK_IT, \
      .level            = __testbench_parent_context->level + 1, \
      .setup            = __testbench_parent_context->setup, \
      .setup_udata    = __testbench_parent_context->setup_udata, \
      .teardown         = __testbench_parent_context->teardown, \
      .teardown_udata = __testbench_parent_context->teardown_udata \
    }; \
    __testbench_global_context->block_context = &__testbench_local_context; \
    bool __testbench_pass = true; \
    char __testbench_error[TESTBENCH_ERROR_STRING_MAX_LEN] = ""; \
    int __testbench_control_pipe[2], __testbench_output_pipe[2]; \
    pipe(__testbench_control_pipe); \
    pipe(__testbench_output_pipe); \
    pid_t __testbench_fork_pid = fork(); \
    if(__testbench_fork_pid == 0) { \
      close(__testbench_control_pipe[0]); \
      close(__testbench_output_pipe[0]); \
      int __testbench_putput_pipe_1_dup = dup2(__testbench_output_pipe[1], 1); \
      int __testbench_putput_pipe_2_dup = dup2(__testbench_output_pipe[1], 2); \
      fcntl(__testbench_putput_pipe_1_dup, F_SETFL, \
          fcntl(__testbench_putput_pipe_1_dup, F_GETFL) | O_NONBLOCK); \
      fcntl(__testbench_putput_pipe_2_dup, F_SETFL, \
          fcntl(__testbench_putput_pipe_2_dup, F_GETFL) | O_NONBLOCK); \
      void * fixtures = __testbench_local_context.setup ? \
        __testbench_local_context.setup(__testbench_local_context.setup_udata) : NULL; \
      jmp_buf __testbench_jmp_buf; \
      int __testbench_jmp_res = setjmp(__testbench_jmp_buf); \
      if (__testbench_jmp_res == 0) { \
        __it_block; \
      } else if (__testbench_jmp_res == 1) { \
        __testbench_pass = false; \
      } \
      close(__testbench_output_pipe[1]); \
      __testbench_local_context.teardown ? \
          __testbench_local_context.teardown(__testbench_local_context.teardown_udata, fixtures) : \
          NULL; \
      write(__testbench_control_pipe[1], &__testbench_error, TESTBENCH_ERROR_STRING_MAX_LEN); \
      exit(!__testbench_pass); \
    } else { \
      close(__testbench_control_pipe[1]); \
      close(__testbench_output_pipe[1]); \
      read(__testbench_control_pipe[0], &__testbench_error, TESTBENCH_ERROR_STRING_MAX_LEN); \
      int __testbench_child_exit_code; \
      waitpid(__testbench_fork_pid, &__testbench_child_exit_code, 0); \
      __testbench_pass = !__testbench_child_exit_code; \
    } \
    ++(__testbench_global_context->total); \
    if (!__testbench_pass) { \
      ++(__testbench_global_context->failed); \
      __TESTBENCH_PRINT( \
          TESTBENCH_ANSI_COLOR_RED "✗ " \
          TESTBENCH_ANSI_COLOR_LIGHT_GRAY __name TESTBENCH_ANSI_RESET); \
      if (__testbench_error[0] != '\0') { \
        __TESTBENCH_PRINT( \
            TESTBENCH_ANSI_COLOR_RED "  %s" TESTBENCH_ANSI_RESET, (char *) &__testbench_error); \
         _testbench_print_error(__testbench_output_pipe[0], __testbench_local_context.level); \
      } \
    } else { \
      __TESTBENCH_PRINT( \
          TESTBENCH_ANSI_COLOR_GREEN "✓ " \
          TESTBENCH_ANSI_COLOR_DARK_GRAY __name TESTBENCH_ANSI_RESET); \
    } \
    __testbench_global_context->block_context = __testbench_parent_context; \
  }

/**
 * Print the results, and return the program exit code.
 */
#define RESULTS() \
  ({ \
    if (!__testbench_global_context->failed) { \
      printf(TESTBENCH_ANSI_BOLD TESTBENCH_ANSI_COLOR_GREEN \
          "\n✓ %u test%s complete.\n\n" TESTBENCH_ANSI_RESET, \
          __testbench_global_context->total, __testbench_global_context->total == 1 ? "" : "s"); \
    } else { \
      printf(TESTBENCH_ANSI_BOLD TESTBENCH_ANSI_COLOR_RED \
          "\n✗ %u test%s out of %u failed.\n\n" TESTBENCH_ANSI_RESET, \
          __testbench_global_context->failed, __testbench_global_context->failed == 1 ? "" : "s", \
          __testbench_global_context->total); \
    } \
    !!__testbench_global_context->failed; \
  })


/******************************************************************************
 * FIXTURES MACROS
 */

/**
 * Set the setup function, in the current context.
 */
#define SETUP(__fn, __udata) \
  ({ \
    __testbench_global_context->block_context->setup = (__fn); \
    __testbench_global_context->block_context->setup_udata = (__udata); \
  })

/**
 * Set the teardown function, in the current context.
 */
#define TEARDOWN(__fn, __udata) \
  ({ \
    __testbench_global_context->block_context->teardown = (__fn); \
    __testbench_global_context->block_context->teardown_udata = (__udata); \
  })


/******************************************************************************
 * BASIC ASSERTION MACROS
 */

/**
 * Consider the current test as a success, not execuring any further instructions.
 */
#define PASS() longjmp(__testbench_jmp_buf, 2)

/**
 * Consider the current test as a failure, not execuring any further instructions, with a custom
 * formatted error message.
 */
#define FAIL_DESC(__format, ...) \
  ({ \
    snprintf((char *) &__testbench_error, TESTBENCH_ERROR_STRING_MAX_LEN, __format " -- %s:%i", \
        ## __VA_ARGS__, __FILE__, __LINE__); \
    longjmp(__testbench_jmp_buf, 1); \
  })

/**
 * Consider the current test as a success, not execuring any further instructions, with a default
 * error message.
 */
#define FAIL() FAIL_DESC("FAIL()")

/**
 * Expect `__what` to be true, with a default error message in case of failure.
 */
#define ASSERT(__what) \
  if (!(__what)) { \
    FAIL_DESC("ASSERT()"); \
  }

/**
 * Expect `__what` to be true, with a custom formatted error message in case of failure.
 */
#define ASSERT_DESC(__what, __format, ...) \
  if (!(__what)) { \
    FAIL_DESC(__format, ## __VA_ARGS__); \
  }


/*****************************************************************************/


#ifdef __cplusplus
} // extern "C"
#endif
