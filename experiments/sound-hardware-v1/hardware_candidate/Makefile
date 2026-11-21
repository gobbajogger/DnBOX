CC ?= cc
CFLAGS := -std=c11 -O2 -ffp-contract=off -fno-fast-math -DX0X_HOST -Wall -Wextra -Werror
CPPFLAGS := -I src
SOURCES := src/dnbx_core.c

.PHONY: all test sanitize evidence clean
all: build/test_core build/libdnbx.so build/bench_core
build:
	mkdir -p build
build/dnbx_core.o: $(SOURCES) src/dnbx_core.h src/fastmath.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -fstack-usage -c $< -o $@
build/libdnbx.so: build/dnbx_core.o
	$(CC) -shared $< -o $@
build/test_core: tests/test_core.c build/dnbx_core.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -lm -o $@
build/test_live_controls: tests/test_live_controls.c build/dnbx_core.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
build/bench_core: tests/bench_core.c build/dnbx_core.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@
test: build/test_core build/test_live_controls
	./build/test_core evidence/candidate_amp_trace.csv
	./build/test_live_controls
sanitize: | build
	$(CC) $(CPPFLAGS) -std=c11 -O1 -g -ffp-contract=off -fno-fast-math -DX0X_HOST -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer $(SOURCES) tests/test_core.c -lm -o build/test_core_sanitize
	ASAN_OPTIONS=detect_leaks=0 ./build/test_core_sanitize
evidence: all
	python3 tests/assess_evidence.py
clean:
	rm -rf build
