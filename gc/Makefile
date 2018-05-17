HEAD              := $(wildcard *.hh)
SRC               := $(wildcard *.cc)
OBJS              := $(addsuffix .o,$(basename $(SRC) ))
SRCS_OT           := $(wildcard OTExtension/ENCRYPTO_utils/*.cpp) \
                     $(wildcard OTExtension/ENCRYPTO_utils/crypto/*.cpp) \
                     $(wildcard OTExtension/ot/*.cpp)
OBJS_OT           := $(addsuffix .o,$(basename $(SRCS_OT)))
MIRACL_LIB        := OTExtension/ENCRYPTO_utils/miracl_lib/miracl.a

CC                := gcc
CXX               := g++
CXXFLAGS          := -std=c++11 -maes -msse3 -pthread -O1 -Wno-ignored-attributes -fPIC
LDFLAGS           := -lpthread -lssl -lcrypto -lgmp -lgmpxx
CLANG-FORMAT      := clang-format

.PHONY: all cscope clang-format clean

all: otext libgash_gc.a

# debug:
# 	@ echo $(SRCS_OT)
# 	@ echo $(OBJS_OT)

libgash_gc.a: $(OBJS) $(OBJS_OT)
	@ echo "    Building $@"
ifeq ("$(wildcard $(MIRACL_LIB))","")
	@ ./build_miracl.sh
endif
	@ ar rvs $@ $^ $(MIRACL_LIB)
	@ mkdir -p ../build/lib && cp $@ ../build/lib

# libgash_gc.so: $(OBJS)
# 	@ echo "    Building $@"
# 	@ $(CXX) -shared -o $@ -fPIC $^
# 	@ mkdir -p ../build/lib && cp $@ ../build/lib

otext:
	@ echo "     Building OTExtension"
	@ cd OTExtension && make

%.o: %.cc
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
