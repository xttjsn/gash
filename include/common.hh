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

#include <assert.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <emmintrin.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <math.h>
#include <set>
#include <stack>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <gmpxx.h>

#define WARNING(str) std::cerr << str << std::endl

#define FATAL(str)                                                             \
  std::cerr << str << std::endl;                                               \
  abort()

#define REQUIRE_NOT_NULL(expr)                                                 \
  do {                                                                         \
    if (expr == NULL) {                                                        \
      FATAL(__FILE__ << ": " << __LINE__ << ": " << #expr << " is NULL");      \
    }                                                                          \
  } while (0)

#define NOT_YET_IMPLEMENTED(f) FATAL("Not yet implemented error:" << f)

#define GASSERT(expr)                                                          \
  do {                                                                         \
    if (!(expr)) {                                                             \
      FATAL(__FILE__ << ": " << __LINE__ << ": " << #expr << " failed");       \
    }                                                                          \
  } while (0)

/**
 * Get the n-th bit of v
 */
#define getbit(v, n) (v >> n) & 1

#define clearbit(v, n) (v &= ~(1 << n))

using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::ifstream;
using std::make_pair;
using std::map;
using std::ostream;
using std::pair;
using std::set;
using std::string;
using std::to_string;
using std::unordered_map;
using std::vector;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define RSA_bits 2048
#define LABELSIZE 16 // 16 bytes = 128 bits
// #define GASH_DEBUG true                  // Comment this to toggle debug
// messages #define GASH_NO_OT                          // Uncomment to use IKNP
// OT
#define GASH_TIMER  // Comment this to toggle timer
#define GASH_GC_GRR // Enable garbled row reduction
#define getZEROblock() _mm_setzero_si128()

/* Errors */
#define G_EEXIST 0x1
#define G_ENOENT 0x2
#define G_ETCP 0x3
#define G_EINVAL 0x4

#define TABx1 "    "
#define TABx2 "        "

#define OT_PORT 23443

#define funcXOR 6
#define funcAND 8
#define funcOR 14

#define xor_magic_num 0x10179394
#define nonxor_magic_num 0x00000639

#define CONC2(x, y) x##y
#define CONC1(x, y) CONC2(x, y)
#define NEWVAR(x) CONC1(x, __COUNTER__)

#define REQUIRE_GOOD_STATUS_IMPL(expr, var)                                    \
  int var;                                                                     \
  if ((var = (expr)) < 0)                                                      \
  return var

#define REQUIRE_GOOD_STATUS(expr)                                              \
  do {                                                                         \
    REQUIRE_GOOD_STATUS_IMPL(expr, NEWVAR(stat));                              \
  } while (0)

namespace gashlang {
    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;
    typedef int8_t i8;
    typedef int16_t i16;
    typedef int32_t i32;
    typedef int64_t i64;
}

namespace gashgc {

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef __m128i block;

typedef vector<int> IntVec;
typedef vector<block> LabelVec;

} // namespace gashgc

#endif
