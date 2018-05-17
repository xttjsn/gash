BUILD_DIR            := ../../build
GC_LIB               := $(BUILD_DIR)/lib/libgash_gc.a
LANG_LIB             := $(BUILD_DIR)/lib/libgash_lang.a
GC_SLIB              := $(BUILD_DIR)/lib/libgash_gc.so
LANG_SLIB            := $(BUILD_DIR)/lib/libgash_lang.so
SRC                  := $(wildcard *.cc)
BIN                  := $(addprefix test_,$(basename $(SRC)))

CXX                  := g++
CXXFLAGS             := -std=c++11 -pthread -Wno-ignored-attributes
LDFLAGS              := -lpthread -lgtest

all: $(BIN)

test_%: %.cc
	@ echo "    Compiling \"$<\""
	@ $(CXX) $(CXXFLAGS) -o $@ $< $(LANG_LIB) $(GC_LIB) $(LDFLAGS)

clean:
	@ rm -f test_*

.PHONY: all clean
