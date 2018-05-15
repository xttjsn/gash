/*
 * aes.h -- Aes encryption API
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

#ifndef GASH_AES_H
#define GASH_AES_H

#include <wmmintrin.h>
#include "../include/common.hh"

namespace gashgc {

  /**
   * Expand the initial key to round encryption keys
   *
   * @param key
   * @param rndkeys
   */
  void expand_key128(__m128i key, __m128i* rndkeys);

  /**
   * Expand the initial key to round decryption keys
   *
   * @param key
   * @param rndkeys
   */
  void expand_deckey128(__m128i key, __m128i* rndkeys);

  /**
   * Encrypt a 128-bit block
   *
   * @param msg
   * @param key
   *
   * @return
   */
  block aes_encrypt128(block msg, block key);

  /**
   * Decrypt a 128-bit block
   *
   * @param cipher
   * @param key
   *
   * @return
   */
  block aes_decrypt128(block cipher, block key);

  /**
   * Get a ret?
   * NOTE: consider remove this function
   *
   * @param a
   * @param b
   * @param T
   * @param key
   *
   * @return
   */
  block getRet(block a, block b, block T, block key);

  /**
   * Encrypt a gate with label 'a', label 'b', tweek 'T' and label 'c'
   *
   * @param a
   * @param b
   * @param T
   * @param c
   * @param key
   *
   * @return
   */
  block encrypt(block a, block b, block T, block c, block key);

  /**
   * Decrypt a gate with label 'a', label 'b', tweek 'T' and cihertext 'cipher'
   *
   * @param a
   * @param b
   * @param T
   * @param cipher
   * @param key
   *
   * @return
   */
  block decrypt(block a, block b, block T, block cipher, block key);

}

#endif
