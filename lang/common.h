/*
 * common.h -- Common includes and macro definitions
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

#ifndef GASH_COMMON_H
#define GASH_COMMON_H

#include <stdlib.h>
#include <iostream>
#include <stdarg.h>
#include <vector>
#include <string>
#include <utility>
#include <math.h>
#include <assert.h>
#include <iomanip>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <unordered_map>
#include <stack>

#define WARNING(str)                                  \
  std::cerr << str << std::endl

#define FATAL(str)                                    \
  std::cerr << str << std::endl;                      \
  abort()


using std::vector;
using std::map;
using std::unordered_map;
using std::pair;
using std::cout;
using std::endl;
using std::string;
using std::ostream;
using std::make_pair;

namespace gashlang {

  typedef uint32_t u32;
  typedef uint64_t u64;
  typedef int32_t i32;
  typedef int64_t i64;

  typedef enum {
    AND = 8,
    OR = 14,
    XOR = 6,
    DFF = 16
  } Op;

  typedef vector<u64> Tuple;

}  // gashlang

#endif
