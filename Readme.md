# Testbench

(Damn-)simple block-testing framework for C, based on macros, with a sexy eye-candy syntax.

## Installation

Testbench is a header-only library. Just include ```testbench.h``` in the header of your test file, and you're basically
done.

## Usage

A basic test program would look like this:

### Getting started

```c
#include <testbench.h>

int main() {

  DESCRIBE("Program", {
    DESCRIBE(".func()", {
      IT("should work", {
        ASSERT(42 == 40 + 2);
      })
    })
  })

  return RESULTS();
}
```

When running, it should give you an output like this:

```
$ ./testbench_test

Program
  .func()
    ✓ should work

✓ 1 test complete.
```

### Better logging

The default logging for failures gives you the file where the failing test is, and the line that has resulted in a
failure. If you want some more debugging upon failure, this may be a little insufficient. A solution is to use the
```ASSERT_DESC``` macro:

```c
#include <testbench.h>

int main() {

  DESCRIBE("Program", {
    DESCRIBE(".func()", {
      IT("should work", {
        ASSERT_DESC(false, "This is an %s message", "interesting");
      })
    })
  })

  return RESULTS();
}
```

When running, the output is much more helpful (well... not in that example, but it could be):

```
$ ./testbench_test

Program
  .func()
    ✗ should work
      This is an interesting message -- testbench_test.c:8

✗ 1 test out of 1 failed.
```

### And more

Testbench also supports setup and teardown functions for fixtures, and test functions. See the `testbench_test.c`
file for usage examples.


## Note

Testbench is an early (very early) release, and though we are quite confident that it is functional at the time, it may
not be exempt of bugs, and its API could change at any time.

## License

Testbench is made available under the terms of the GNU Lesser General Public License version 3, as stated in the
`LICENSE` file.
