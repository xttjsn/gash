CLANG-FORMAT            := clang-format

all: api

cmpl:
	@ cd cmpl && make

grbl:
	@ cd grbl && make

tcp:
	@ cd tcp && make

ot:
	@ cd ot && make

exec:
	@ cd exec && make

res:
	@ cd res && make

api:
	@ cd api && make

clean:
	@ cd cmpl && make clean

clang-format:
	@ echo "Doing clang-format"
	@ $(foreach src,$(wildcard */*.cc),$(CLANG-FORMAT) -i -style=file $(src);)
	@ echo "Format finished"

.PHONY: all cmpl grbl tcp ot exec res api
