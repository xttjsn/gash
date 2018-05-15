/*
 * aes.cc -- Implementation of AES API
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

#include "aes.hh"

#define EXPAND_ROUND(prev, v0, v1, v2, v3, v4, rcon)                  \
  v0 = _mm_aeskeygenassist_si128(prev, rcon);                         \
  v1 = _mm_setzero_si128();                                           \
  v1 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(v1),          \
                                       _mm_castsi128_ps(prev), 64));  \
  v2 = _mm_xor_si128(prev, v1);                                       \
  v3 = _mm_castps_si128(_mm_shuffle_ps(_mm_castsi128_ps(v1),          \
                                       _mm_castsi128_ps(v2), 152));   \
  v3 = _mm_xor_si128(v3, v2);                                         \
  v4 = _mm_xor_si128(v3, _mm_shuffle_epi32(v0, 255))

namespace gashgc {

  static const u8 AESkey_buf[64] =
    {
      105, 187, 117, 137,
      65, 140, 104, 185,
      74, 49, 246, 13,
      246, 67, 88, 55,
      229, 178, 210, 121,
      195, 44, 80, 195,
      184, 111, 175, 138,
      56, 18, 248, 26,
      203, 123, 54, 238,
      95, 101, 16, 236,
      161, 238, 111, 155,
      74, 190, 115, 80,
      127, 40, 15, 186,
      77, 65, 117, 27,
      96, 74, 94, 115,
      18, 77, 79, 132
    };

  block AESkey = _mm_load_si128((const __m128i *) AESkey_buf);

    void expand_key128(__m128i key, __m128i *rndkeys) {
        rndkeys[0] = key;
        __m128i v0, v1, v2, v3;
        EXPAND_ROUND(rndkeys[0], v0, v1, v2, v3, rndkeys[1], 1);
        EXPAND_ROUND(rndkeys[1], v0, v1, v2, v3, rndkeys[2], 2);
        EXPAND_ROUND(rndkeys[2], v0, v1, v2, v3, rndkeys[3], 4);
        EXPAND_ROUND(rndkeys[3], v0, v1, v2, v3, rndkeys[4], 8);
        EXPAND_ROUND(rndkeys[4], v0, v1, v2, v3, rndkeys[5], 16);
        EXPAND_ROUND(rndkeys[5], v0, v1, v2, v3, rndkeys[6], 32);
        EXPAND_ROUND(rndkeys[6], v0, v1, v2, v3, rndkeys[7], 64);
        EXPAND_ROUND(rndkeys[7], v0, v1, v2, v3, rndkeys[8], 128);
        EXPAND_ROUND(rndkeys[8], v0, v1, v2, v3, rndkeys[9], 27);
        EXPAND_ROUND(rndkeys[9], v0, v1, v2, v3, rndkeys[10], 54);
    }

    void expand_deckey128(__m128i key, __m128i *rndkeys) {
        __m128i tmp[11];
        expand_key128(key, tmp);
        int i = 10;
        int j = 0;
        rndkeys[i--] = tmp[j++];
        while (i) {
            rndkeys[i--] = _mm_aesimc_si128(tmp[j++]);
        }
        rndkeys[i] = tmp[j];
    }

    void aes_encrypt128(unsigned char *msg, unsigned char *key, unsigned char *out) {
        __m128i userkey = _mm_load_si128((__m128i *) key);
        __m128i rndkeys[11];
        expand_key128(userkey, rndkeys);
        __m128i state = _mm_load_si128((__m128i *) msg);
        state = _mm_xor_si128(rndkeys[0], state);        // AddRoundKey
        int i;
        for (i = 1; i < 10; i++) {
            state = _mm_aesenc_si128(state, rndkeys[i]);
        }
        state = _mm_aesenclast_si128(state, rndkeys[i]);
        _mm_store_si128((__m128i *) out, state);
    }

    void aes_decrypt128(unsigned char *ciphertext, unsigned char *key, unsigned char *out) {
        __m128i userkey = _mm_load_si128((__m128i *) key);
        __m128i rndkeys[11];
        expand_deckey128(userkey, rndkeys);
        __m128i state = _mm_load_si128((__m128i *) ciphertext);
        state = _mm_xor_si128(rndkeys[0], state);        // AddRoundKey
        int i;
        for (i = 1; i < 10; i++) {
            state = _mm_aesdec_si128(state, rndkeys[i]);
        }
        state = _mm_aesdeclast_si128(state, rndkeys[i]);
        _mm_store_si128((__m128i *) out, state);
    }

    block aes_encrypt128(block msg, block key) {
        __m128i userkey = key;
        __m128i rndkeys[11];
        expand_key128(userkey, rndkeys);
        __m128i state = msg;
        state = _mm_xor_si128(rndkeys[0], state);        // AddRoundKey
        int i;
        for (i = 1; i < 10; i++) {
            state = _mm_aesenc_si128(state, rndkeys[i]);
        }
        state = _mm_aesenclast_si128(state, rndkeys[i]);
        return state;
    }

    block aes_decrypt128(block cipher, block key) {
        __m128i userkey = key;
        __m128i rndkeys[11];
        expand_deckey128(userkey, rndkeys);
        __m128i state = cipher;
        state = _mm_xor_si128(rndkeys[0], state);        // AddRoundKey
        int i;
        for (i = 1; i < 10; i++) {
            state = _mm_aesdec_si128(state, rndkeys[i]);
        }
        state = _mm_aesdeclast_si128(state, rndkeys[i]);
        return state;
    }

    block getRet(block a, block b, block T, block key) {
        block ret;
        block K = _mm_xor_si128(_mm_xor_si128(_mm_slli_epi64(a, 1),
                                              _mm_slli_epi64(b, 2)), T);
        ret = aes_encrypt128(K, key);
        return ret;
    }

    block encrypt(block a, block b, block T, block c, block key) {
        // Enc(X^a, X^b, T, X^c) = \pi(K) ^ K ^ X^c,
        // K = 2X^a ^ 4X^b ^ T
        block ret;
        block K = _mm_xor_si128(_mm_xor_si128(_mm_slli_epi64(a, 1),
                                              _mm_slli_epi64(b, 2)), T);
        ret = aes_encrypt128(K, key);
        ret = _mm_xor_si128(_mm_xor_si128(ret, K), c);
        return ret;
    }

    block decrypt(block a, block b, block T, block cipher, block key) {
        // X^c =  \pi(K) ^ K ^ Enc(...)
        block ret;
        block K = _mm_xor_si128(_mm_xor_si128(_mm_slli_epi64(a, 1),
                                              _mm_slli_epi64(b, 2)), T);
        ret = aes_decrypt128(K, key);
        ret = _mm_xor_si128(_mm_xor_si128(ret, K), cipher);
        return ret;
    }
}
