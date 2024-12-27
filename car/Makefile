TESTSUITE_DIR := /testsuite

TESTS := $(addprefix $(TESTSUITE_DIR)/test,1 2 3 4 5 6 7 8 9)

all:
	gcc -o gps_uploader -g -fsanitize=undefined -fno-sanitize-recover src/gps_uploader.c

# Run all tests in the testsuite
# Usage: make test
# This will execute each test and display the output.
test: $(TESTS)
	@for test in $^; do \
		echo "- Running $$test..."; \
		./gps_uploader  $$test; \
	done

# Upload test results to the specified API endpoint
# Usage: make upload
# This will execute each test and send the results to the API.
upload: $(TESTS)
	@for test in $^; do \
		echo "- Running $$test..."; \
		./gps_uploader  $$test http://api:8000/location; \
	done

# Clean up generated files
# Usage: make clean
# This will remove the compiled binary and any temporary files.
clean:
	rm -f gps_uploader
