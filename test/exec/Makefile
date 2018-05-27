BUILD_DIR            := ../../build
GC_LIB               := $(BUILD_DIR)/lib/libgash_gc.a
LANG_LIB             := $(BUILD_DIR)/lib/libgash_lang.a
MIRACL_LIB           := $(BUILD_DIR)/lib/libmiracl.a
GASH_SLIB            := $(BUILD_DIR)/lib/libgash.so
SRC                  := $(wildcard *.cc)
BIN                  := $(addprefix test_,$(basename $(SRC)))

CXX                  := g++
CXXFLAGS             := -std=c++11 -pthread -Wno-ignored-attributes -g -fPIC
# LDFLAGS              := -lpthread -lgtest -L$(BUILD_DIR)/lib -lgash -lgmp -lgmpxx -lcrypto
LDFLAGS              := -lpthread -lgtest $(GC_LIB) $(LANG_LIB) $(MIRACL_LIB) -lgmp -lgmpxx -lcrypto
CLANG-FORMAT         := clang-format

all: $(BIN)

test_%: %.cc $(GASH_SLIB)
	@ echo "    Compiling \"$<\""
	@ echo "$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)"
	@ $(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(GASH_SLIB):
	# Empty

clang-format:
	@ echo "Doing clang-format"
	@ $(foreach src,$(SRC),$(CLANG-FORMAT) -i -style=file $(src);)
	@ echo "Format finished"

clean:
	@ rm -f test_*
	@ rm *.circ *.dat

.PHONY: all clean
