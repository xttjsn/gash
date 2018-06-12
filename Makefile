BUILD_DIR      := build
GC_LIB         := $(BUILD_DIR)/lib/libgash_gc.a
LANG_LIB       := $(BUILD_DIR)/lib/libgash_lang.a
MIRACL_LIB     := $(BUILD_DIR)/lib/libmiracl.a
RES_LIB        := $(BUILD_DIR)/lib/libgash_res.a
API_LIB        := $(BUILD_DIR)/lib/libgash_api.a
GASH_SLIB      := $(BUILD_DIR)/lib/libgash.so
HEAD           := $(parser_HEAD) $(lexer_HEAD) $(wildcard *.hh)
SRC            := $(parser_CSRC) $(lexer_CSRC) $(filter-out main.cc, $(wildcard *.cc))
OBJS           := $(addsuffix .o,$(basename $(SRC)))
INSTALL_PREFIX := /usr/local

CXX            := g++
CXXFLAGS       := -std=c++11 -pthread -Wno-ignored-attributes -g -fPIC

all: gc lang res api $(GASH_SLIB) test

gc $(GC_LIB) $(MIRACL_LIB):
	@ cd gc && make

lang $(LANG_LIB):
	@ cd lang && make

res $(RES_LIB):
	@ cd res && make

api $(API_LIB):
	@ cd api && make

$(GASH_SLIB): $(LANG_LIB) $(GC_LIB) $(MIRACL_LIB) $(RES_LIB)
	@ echo "    Building libgash.so"
	@ echo "$(CXX) $^ -shared -o $@"
	@ $(CXX) -g -o $@ -shared -Wl,--whole-archive $^ -Wl,--no-whole-archive

test:
	@ echo "    Building tests"
	@ cd test && make

install: $(GASH_SLIB)
	@ echo "cp $(GASH_SLIB) $(INSTALL_PREFIX)/lib"
	@ cp $(GASH_SLIB) $(INSTALL_PREFIX)/lib

cscope:
	@ find . -name "*.[ch]" > cscope.files
	@ find . -name "*.cc" >> cscope.files
	@ find . -name "*.hh" >> cscope.files
	@ cscope -b -q -k

clang-format:
	@ cd lang && $(MAKE) clang-format
	@ cd gc && $(MAKE) clang-format

clean:
	@ cd lang && $(MAKE) clean
	@ cd gc && $(MAKE) clean

.PHONY: all gc lang api res test clean cscope clang-format
