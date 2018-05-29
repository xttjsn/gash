/*
 * util.h -- Utility classes and methods
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

#ifndef GASH_GC_UTIL_H
#define GASH_GC_UTIL_H

#include "../include/common.hh"
#include <time.h>

namespace gashgc {

  #define is_odd(n) (!(n % 2 == 0))

  /**
   * Event
   *
   */
  class Event {
  public:

    string m_name;

    u32 m_elapsed_time;

  Event(string name, u32 elapsed_time) : m_name(name), m_elapsed_time(elapsed_time) {}

  };

  /**
   * Timer
   *
   */
  typedef map<int, Event> EventMap;
  class Timer {

    EventMap m_records;

    u32 m_nrecords = 0;

    bool m_ticking = 0;

    u32 m_elapsed_time = 0;

    timespec m_last_time;

    timespec m_curr_time;

    const char* m_curr_name;

  public:

    void tic(const char*);

    void toc();

    void report();

    Timer(){}
  };

  /**
   * String related
   *
   */
  typedef vector<string> StringVec;

  void split(string str, const char* delim, StringVec& ret);

  /**
   * PRNG related
   *
   */
  void srand_sse(u32 seed);

  /**
   * Block related
   *
   */

  /**
   * Generate a random block
   *
   * @return
   */
  block random_block();

  /**
   * Generate a random block, with the constrain that the LSB is 1
   *
   *
   * @return
   */
  block random_R();

  /**
   * Test whether two blocks are equal
   *
   * @param a
   * @param b
   *
   * @return
   */
  bool block_eq(block a, block b);

  /**
   * Convert block to hexdecimal
   *
   * @param a
   *
   * @return
   */
  string block2hex(block a);

  /**
   * Convert a block to decimal
   *
   * @param a
   *
   * @return
   */
  string block2dec(block a);

  /**
   * Evaluate a binary gate
   *
   * @param a
   * @param b
   * @param func
   *
   * @return
   */
  u32 eval_bgate(int a, int b, int func);

  /**
   * Evaluate a binary gate, with invert option
   *
   * @param a
   * @param b
   * @param func
   * @param inverted
   *
   * @return
   */
  u32 eval_gate(int a, int b, int func, bool inverted);


  /**
   * Get least significant bit
   * Endianness does not matter as long as we use this function consistently
   *
   * @param label
   *
   * @return
   */
  inline int get_lsb(block label) {
    // TODO: test whether get_lsb and set_lsb is correct

    u8 *var = (u8 *) &label;

    return (*(var) & 1) == 1 ? 1 : 0;

  }

  /**
   * Set least significant bit
   *
   * @param lbl
   */
  inline void set_lsb(block& lbl) {
    u8 *p = (u8*)&lbl;
    *p |= 1;
  }

  /**
   * XOR two blocks
   *
   * @param block_a
   * @param block_b
   *
   * @return
   */
  inline block xor_block(block a, block b) {

    return _mm_xor_si128(a, b);

  }

  /**
   * Create a new tweak
   *
   * @param id
   *
   * @return
   */
  block new_tweak(u32 id);

  /**
   * Concatenate least_half and great_half to a block
   *
   * @param least_half
   * @param great_half
   *
   * @return
   */
  inline block conc_u64_to_block(u64& least_half, u64& great_half) {

    return _mm_set_epi64x(great_half, least_half);

  }

  /**
   * Get the first half of a block
   *
   * @param b
   *
   * @return
   */
  inline u64 get_block_first_half(block& b) {

    return *((u64*)&b);

  }

  /**
   * Get the second half of a block
   *
   * @param b
   *
   * @return
   */
  inline u64 get_block_second_half(block& b) {
    return *(((u64*)&b)+1);
  }

  typedef map<u32, pair<block, block>> IdLabelsMap;
  /**
   * Find semantic of the label
   *
   * @param id
   * @param outputMap
   * @param label
   *
   * @return
   */
  int get_lbl_smtc(u32 id, IdLabelsMap& outMap, block& lbl);

  /**
   * Convert the 16 bytes of the range from src + offset to src + offset + 16 to a block
   *
   * @param src
   * @param offset
   * @param lbl
   *
   * @return 0 if success, errno if failure (mostly due to invalid index)
   */
  int byte2label(char* src, u32 offset, block& lbl);

  /**
   * Convert selections to bytes
   * `dest` should be memseted with 0
   *
   * @param select_vec
   * @param dest
   *
   * @return 0 if success, errno if failure
   */
  int select2bytes(IntVec select_vec, char* dest);

  /**
   * Marshal the label vector
   *
   * @param lblvec
   * @param dest
   *
   * @return
   */
  int label_vec_marshal(LabelVec& lblvec, char* dest);

  /**
   * Set bit
   *
   * @param src
   * @param idx
   */
  inline void set_bit(char& src, u32 idx) {
    src |= 1UL << idx;
  }

}

#endif
