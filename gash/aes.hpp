//
//  aes.hpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/17/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef aes_hpp
#define aes_hpp

#include <stdio.h>
#include <wmmintrin.h>

typedef __m128i block;

void expand_key128(__m128i key, __m128i* rndkeys);

void expand_deckey128(__m128i key, __m128i* rndkeys);

void aes_encrypt128(unsigned char* msg, unsigned char* key, unsigned char* out);

void aes_decrypt128(unsigned char* ciphertext, unsigned char* key, unsigned char* out);

block aes_encrypt128(block msg, block key);

block aes_decrypt128(block cipher, block key);

block getRet(block a, block b, block T, block key);

block encrypt(block a, block b, block T, block c, block key);

block decrypt(block a, block b, block T, block cipher, block key);

#endif /* aes_hpp */
