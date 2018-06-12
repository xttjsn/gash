BUILD_DIR            := ../../build
GC_LIB               := $(BUILD_DIR)/lib/libgash_gc.a
LANG_LIB             := $(BUILD_DIR)/lib/libgash_lang.a
RES_LIB              := $(BUILD_DIR)/lib/libgash_res.a
API_LIB              := $(BUILD_DIR)/lib/libgash_api.a
MIRACL_LIB           := $(BUILD_DIR)/lib/libmiracl.a
GASH_SLIB            := $(BUILD_DIR)/lib/libgash.so
SRC                  := $(wildcard *.cc)
BIN                  := $(addprefix test_,$(basename $(SRC)))

CXX                  := g++
CXXFLAGS             := -std=c++11 -pthread -Wno-ignored-attributes -g -fPIC
# LDFLAGS              := -lpthread -lgtest -L$(BUILD_DIR)/lib -lgash -lgmp -lgmpxx -lcrypto
LDFLAGS              := -lpthread -lgtest -L$(BUILD_DIR)/lib $(API_LIB) $(GC_LIB) $(LANG_LIB) $(RES_LIB) $(MIRACL_LIB)  -lgmp -lgmpxx -lcrypto

all: $(BIN)

test_%: %.cc $(GASH_SLIB) $(RES_LIB) $(API_LIB)
	@ echo "    Compiling \"$<\""
	@ echo "$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)"
	@ $(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(GASH_SLIB):
	# Empty

clean:
	@ rm -f test_*

.PHONY: all clean
