CLANG-FORMAT            := clang-format

all: ot

cmpl:
	@ cd cmpl && make

grbl:
	@ cd grbl && make

tcp:
	@ cd tcp && make

ot:
	@ cd ot && make

clean:
	@ cd cmpl && make clean

clang-format:
	@ echo "Doing clang-format"
	@ $(foreach src,$(wildcard */*.cc),$(CLANG-FORMAT) -i -style=file $(src);)
	@ echo "Format finished"

.PHONY: all cmpl grbl tcp ot
