#!/bin/sh

find . -name "*.[ch]" > cscope.files
find . -name "*.cpp" >> cscope.files
find . -name "*.cc" >> cscope.files
cscope -b -q -k
