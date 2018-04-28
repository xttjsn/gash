lexer_SRC      := lexer.l
parser_SRC     := parser.ypp
lexer_CSRC     := $(addsuffix .cc,$(basename $(lexer_SRC)))
parser_CSRC    := $(addsuffix .cc,$(basename $(parser_SRC)))
lexer_HEAD     := $(addsuffix .l.h,$(basename $(lexer_SRC)))
parser_HEAD    := $(addsuffix .tab.h,$(basename $(parser_SRC)))
HEAD           := $(parser_HEAD) $(lexer_HEAD) $(wildcard *.h)
SRC            := $(parser_CSRC) $(lexer_CSRC) main.cc $(wildcard *.cc)
OBJS           := $(addsuffix .o,$(basename $(SRC) ))

CC             := gcc
CXX            := g++
CXXFLAGS       := -std=c++11 -pthread
LDFLAGS        := -lpthread -lfl -lssl -lcrypto -lboost_program_options
CLANG-FORMAT   := clang-format-3.9

.PHONY: all cscope clang-format clean

all: gashlang

gashlang: $(OBJS)
	@ echo "    Building gashlang"
	@ echo $(OBJS)
	@ echo "$(CXX) $^ -o $@ $(LDFLAGS)"
	@ $(CXX) -o $@ $^ $(LDFLAGS)

$(parser_HEAD) $(parser_CSRC): $(parser_SRC)
	@ echo "    Building parser cc source"
	@ bison --defines=$(parser_HEAD) $< -o $(parser_CSRC)

$(lexer_CSRC): $(lexer_SRC) $(parser_HEAD)
	@ echo "    Building lexer cc source"
	@ flex --outfile=$@ $<

%.o: %.cc
	@ echo "    Compiling \"$<\""
	@ echo "$(CXX) $(CXXFLAGS) -c $< -o $@"
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
	@ rm -f $(OBJS)
	@ rm -f $(parser_CSRC)
	@ rm -f $(lexer_CSRC)
	@ rm -f $(parser_HEAD)
