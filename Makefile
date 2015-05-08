.PHONY: test build-test run-test clean

test: build-test run-test clean

build-test:
	@cc -otestbench_test -Iinclude/ test/testbench_test.c

run-test:
	@./testbench_test

clean:
	@rm -f testbench_test
