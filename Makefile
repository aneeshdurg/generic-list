all: list_test
list_test: list_test.c
	gcc list_test.c -o list_test
	gcc list_test.c -DERROR_TEST -o list_error_test 2>/dev/null || echo "\n-----\nERROR_TEST Passed!"

clean:
	rm -rf list_test list_error_test
