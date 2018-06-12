BUILD_DIR         := ../build
HEAD              := $(wildcard *.hh)
SRC               := $(wildcard *.cc)
OBJS              := $(addsuffix .o,$(basename $(SRC) ))

CC                := gcc
CXX               := g++
CXXFLAGS          := -std=c++11 -maes -msse3 -pthread -O1 -Wno-ignored-attributes -fPIC -g
LDFLAGS           := -lpthread -lssl -lcrypto -lgmp -lgmpxx -lboost
CLANG-FORMAT      := clang-format

.PHONY: all cscope clang-format clean

all: libgash_res.a

libgash_res.a: $(OBJS) $(OBJS_OT)
	@ echo "    Building $@"
	@ ar rvs $@ $^
	@ mkdir -p ../build/lib && cp $@ ../build/lib

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
	@ rm -f $(OBJS) $(OBJS_OT) libgash_func.a
