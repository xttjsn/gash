CLANG-FORMAT            := clang-format

all: cmpl

cmpl:
	@ cd cmpl && make

clean:
	@ cd cmpl && make clean

clang-format:
	@ echo "Doing clang-format"
	@ $(foreach src,$(wildcard */*.cc),$(CLANG-FORMAT) -i -style=file $(src);)
	@ echo "Format finished"

.PHONY: all cmpl
