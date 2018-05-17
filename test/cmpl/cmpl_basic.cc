/*
 * cmpl_basic.cc -- Basic testing for the compilation functionality
 *
 * Author: Xiaoting Tang <tang_xiaoting@brown.edu>
 * Copyright: Xiaoting Tang (2018)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/common.hh"
#include "../../lang/gash_lang.hh"

#include <cstdio>

TEST(CMPL_ADD, ADD_1) {

  extern FILE* yyin;

  const char* src=
    "#definput     a    0            "
    "#definput     b    1            "
    "func add(int1 a, int1 b) {      "
    "    return a + b;               "
    "}";

  yyin = std::tmpfile();
  std::fputs(src, yyin);
  std::rewind(yyin);

  int parse_result = yyparse();
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
