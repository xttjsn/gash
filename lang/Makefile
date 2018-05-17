BUILD_DIR      := ../build
GC_LIB         := $(BUILD_DIR)/lib/libgash_gc.a
INCLUDE        := ../include
HEAD           := $(parser_HEAD) $(lexer_HEAD) $(wildcard *.hh)
SRC            := $(parser_CSRC) $(lexer_CSRC) $(filter-out main.cc, $(wildcard *.cc))
OBJS           := $(addsuffix .o,$(basename $(SRC)))

lexer_SRC      := lexer.l
parser_SRC     := parser.ypp
parser_CSRC    := $(addsuffix .cc,$(basename $(parser_SRC)))
parser_HEAD    := $(addsuffix .tab.h,$(basename $(parser_SRC)))
parser_OBJ     := $(addsuffix .o,$(basename $(parser_CSRC)))
lexer_CSRC     := $(addsuffix .cc,$(basename $(lexer_SRC)))
lexer_HEAD     := $(addsuffix .l.h,$(basename $(lexer_SRC)))
lexer_OBJ      := $(addsuffix .o,$(basename $(lexer_CSRC)))

CC             := gcc
CXX            := g++
CXXFLAGS       := -std=c++11 -pthread -Wno-ignored-attributes -g -fPIC
LDFLAGS        := -lpthread -lfl -lssl -lcrypto -lboost_program_options -lgmp -lgmpxx
CLANG-FORMAT   := clang-format


all: $(parser_HEAD) libgash_lang.a gashlang

libgash_lang.a: $(OBJS) $(parser_OBJ) $(lexer_OBJ)
	@ echo "    Building libgash_lang.a"
	@ ar rvs $@ $^
	@ mkdir -p ../build/lib && cp ./libgash_lang.a ../build/lib

gashlang: $(OBJS) $(parser_OBJ) $(lexer_OBJ) main.o
	@ echo "    Building gashlang"
	@ $(CXX) $^ -o $@ $(GC_LIB) $(LDFLAGS)
	@ mkdir -p ../build/bin && cp ./gashlang ../build/bin

$(parser_HEAD) $(parser_CSRC): $(parser_SRC)
	@ echo "    Building parser cc source"
	@ bison --defines=$(parser_HEAD) $(parser_SRC) -o $(parser_CSRC)

$(lexer_CSRC): $(lexer_SRC) $(parser_HEAD)
	@ echo "    Building lexer cc source"
	@ flex --outfile=$@ $<

%.o: %.cc %.hh
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
	@ rm -f main.o
	@ rm -f $(parser_CSRC)
	@ rm -f $(lexer_CSRC)
	@ rm -f $(parser_HEAD)

.PHONY: all cscope clang-format clean
