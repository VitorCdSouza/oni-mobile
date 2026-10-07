# Builds the SimDLL port.
#   make hybrid      build/libsimport.so: port functions + detour installer, for replay against the original
#   make standalone  build/standalone/libSimDLL.so: the whole library, without the original
CXX ?= g++
# -ffp-contract=off: no fused multiply-add, so arm64 builds round exactly like the x86 original.
CXXFLAGS ?= -O2 -g -std=c++17 -fPIC -ffp-contract=off -Wall -Wextra
SRCS := $(wildcard src/*.cpp)
BUILD := ../build

hybrid: $(BUILD)/libsimport.so

$(BUILD)/libsimport.so: $(SRCS) detour/detour.cpp $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SRCS) detour/detour.cpp -ldl

.PHONY: hybrid

STANDALONE_SRCS := $(SRCS) $(wildcard standalone/*.cpp)

standalone: $(BUILD)/standalone/libSimDLL.so

$(BUILD)/standalone/libSimDLL.so: $(STANDALONE_SRCS) $(wildcard src/*.h)
	mkdir -p $(BUILD)/standalone
	$(CXX) $(CXXFLAGS) -shared -Wl,--no-undefined -o $@ $(STANDALONE_SRCS) -lpthread

.PHONY: standalone

# coverage: same as hybrid, instrumented with gcov. After a replay with SIMPORT pointing at
# build/coverage/libsimport.so, `make coverage-report` lists line coverage per port file, which
# shows the branches the recordings never reach.
COVDIR := $(BUILD)/coverage
coverage: $(COVDIR)/libsimport.so

$(COVDIR)/libsimport.so: $(SRCS) detour/detour.cpp $(wildcard src/*.h)
	mkdir -p $(COVDIR)
	cd $(COVDIR) && $(CXX) $(CXXFLAGS) -O0 --coverage -DSIMPORT_COVERAGE -shared -o libsimport.so \
		$(abspath $(SRCS)) $(abspath detour/detour.cpp) -ldl

# Lists coverage per port source file, then the lines no recording reached.
coverage-report:
	cd $(COVDIR) && for f in libsimport.so-*.gcda; do gcov -n "$$f" 2>/dev/null | grep -A1 "^File '.*/src/"; done
	cd $(COVDIR) && for f in libsimport.so-*.gcda; do gcov -t "$$f" 2>/dev/null; done | \
		awk -F: '/Source:/ {src=$$NF} $$1 ~ /#####/ && src ~ /\/src\// {gsub(/^ +/, "", $$2); print src ":" $$2 ":" $$3}' 

.PHONY: coverage coverage-report
