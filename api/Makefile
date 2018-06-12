BUILD_DIR      := ../build
API_LIB         := $(BUILD_DIR)/lib/libgash_api.a
INCLUDE        := ../include
HEAD           := $(wildcard *.hh)
SRC            := $(wildcard *.cc)
OBJS           := $(addsuffix .o,$(basename $(SRC)))


CC             := gcc
CXX            := g++
CXXFLAGS       := -std=c++11 -pthread -Wno-ignored-attributes -g -fPIC
LDFLAGS        := -lpthread -lfl -lssl -lcrypto -lboost_program_options -lgmp -lgmpxx
CLANG-FORMAT   := clang-format


all: $(API_LIB)

$(API_LIB): $(OBJS)
	@ echo "    Building libgash_api.a"
	@ mkdir -p ../build/lib 
	@ ar rvs $@ $^

%.o: %.cc
	@ echo "    Compiling \"$<\""
	@ $(CXX) $(CXXFLAGS) -c $< -o $@

install:
	@ cp ../build/bin/gashlang /usr/local/bin

cscope:
	@ find . -name "*.[ch]" > cscope.files
	@ find . -name "*.cc" >> cscope.files
	@ cscope -b -q -k

clang-format:
	@ echo "Doing clang-format"
	@ $(foreach src,$(SRC),$(CLANG-FORMAT) -i -style=file $(src);)
	@ echo "Format finished"

clean:
	@ rm -f $(OBJS)
	@ rm -f $(API_LIB)

.PHONY: all cscope clang-format clean
