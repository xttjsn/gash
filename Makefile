all: lang gc

.PHONY: all clean cscope clang-format

lang:
	$(MAKE) -C lang

gc:
	$(MAKE) -C gc

cscope:
	@ find . -name "*.[ch]" > cscope.files
	@ find . -name "*.cc" >> cscope.files
	@ cscope -b -q -k

clang-format:
	@ cd lang && $(MAKE) clang-format
	@ cd gc && $(MAKE) clang-format

clean:
	@ cd lang && $(MAKE) clean
	@ cd gc && $(MAKE) clean



