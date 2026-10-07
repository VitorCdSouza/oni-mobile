# Builds the SimDLL port.
#   make hybrid   build/libsimport.so: port functions + detour installer, for replay against the original
CXX ?= g++
# -ffp-contract=off: no fused multiply-add, so arm64 builds round exactly like the x86 original.
CXXFLAGS ?= -O2 -g -std=c++17 -fPIC -ffp-contract=off -Wall -Wextra
SRCS := $(wildcard src/*.cpp)
BUILD := ../build

hybrid: $(BUILD)/libsimport.so

$(BUILD)/libsimport.so: $(SRCS) detour/detour.cpp $(wildcard src/*.h)
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SRCS) detour/detour.cpp -ldl

.PHONY: hybrid
