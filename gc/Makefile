BUILD_DIR         := ../build
HEAD              := $(wildcard *.hh)
SRC               := $(wildcard *.cc)
OBJS              := $(addsuffix .o,$(basename $(SRC) ))
SRCS_OT           := $(wildcard OTExtension/ENCRYPTO_utils/*.cpp) \
                     $(wildcard OTExtension/ENCRYPTO_utils/crypto/*.cpp) \
                     $(wildcard OTExtension/ot/*.cpp)
OBJS_OT           := $(addsuffix .o,$(basename $(SRCS_OT)))
MIRACL_OLD_PATH   := OTExtension/ENCRYPTO_utils/miracl_lib/miracl.a
MIRACL_LIB        := $(BUILD_DIR)/lib/libmiracl.a
GC_LIB            := $(BUILD_DIR)/lib/libgash_gc.a

CC                := gcc
CXX               := g++
CXXFLAGS          := -std=c++11 -maes -msse3 -pthread -O1 -Wno-ignored-attributes -fPIC -g
LDFLAGS           := -lpthread -lssl -lcrypto -lgmp -lgmpxx -lboost
CLANG-FORMAT      := clang-format

.PHONY: all cscope clang-format clean

all: otext $(GC_LIB)

$(GC_LIB): $(OBJS) $(OBJS_OT)
	@ echo "    Building $@"
	@ mkdir -p ../build/lib
ifeq ("$(wildcard $(MIRACL_LIB))","")
	@ ./build_miracl.sh
	@ mkdir -p ../build/lib && cp $(MIRACL_OLD_PATH) $(MIRACL_LIB)
endif
	@ ar rvs $@ $^

otext:
	@ echo "     Building OTExtension"
	@ sed -i '/CC=g++/c\CC=g++ -fPIC -g' OTExtension/Makefile
	@ cd OTExtension && make

%.o: %.cc %.hh
	@ echo "    Compiling \"$<\""
	@ $(CXX) $(CXXFLAGS) -c $< -o $@

cscope:
	@ find . -name "*.[ch]" > cscope.files
	@ find . -name "*.cc" >> cscope.files
	@ cscope -b -q -k

clang-format:
	@ echo "Doing clang-format"
	@ $(foreach src,$(SRC),$(CLANG-FORMAT) -i -style=file $(src);)
	@ echo "Format finished"

clean:
	@ rm -f $(OBJS) $(OBJS_OT) libgash_gc.a libgash_gc.so
	@ cd OTExtension && make clean
