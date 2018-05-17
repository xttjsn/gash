all: gc lang

gc:
	@ cd gc && make

lang:
	@ cd lang && make

cscope:
	@ find . -name "*.[ch]" > cscope.files
	@ find . -name "*.cc" >> cscope.files
	@ find . -name "*.hh" >> cscope.files
	@ cscope -b -q -k

clang-format:
	@ cd lang && $(MAKE) clang-format
	@ cd gc && $(MAKE) clang-format

clean:
	@ cd lang && $(MAKE) clean
	@ cd gc && $(MAKE) clean

.PHONY: all gc lang clean cscope clang-format
