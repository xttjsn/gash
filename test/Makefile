CLANG-FORMAT            := clang-format

all: cmpl grbl tcp

cmpl:
	@ cd cmpl && make

grbl:
	@ cd grbl && make

tcp:
	@ cd tcp && make

clean:
	@ cd cmpl && make clean

clang-format:
	@ echo "Doing clang-format"
	@ $(foreach src,$(wildcard */*.cc),$(CLANG-FORMAT) -i -style=file $(src);)
	@ echo "Format finished"

.PHONY: all cmpl grbl tcp
