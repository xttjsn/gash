//
//  util.cpp
//  gash
//
//  Created by Xiaoting Tang on 6/18/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#include <stdio.h>
#include "util.hpp"

static block m_seed;

void srand_sse(int seed) {
    m_seed = _mm_set_epi32((int)seed, (int)seed * 3, (int)seed * 7, (int)seed * 11);
}

block random_block() {
    block seed_split;
    block multiplier;
    block adder;
    block mod_mask;
    // block sra_mask;
    // block sseresult;
    static const unsigned int mult[4] = { 214013, 17405, 214013, 69069 };
    static const unsigned int gadd[4] = { 2531011, 10395331, 13737667, 1 };
    static const unsigned int mask[4] = { 0xFFFFFFFF, 0, 0xFFFFFFFF, 0 };
    // static const unsigned int masklo[4] = { 0x00007FFF, 0x00007FFF, 0x00007FFF, 0x00007FFF };
    
    adder = _mm_load_si128((block*)gadd);
    multiplier = _mm_load_si128((block*)mult);
    mod_mask = _mm_load_si128((block*)mask);
    // sra_mask = _mm_load_si128( (block*) masklo);
    seed_split = _mm_shuffle_epi32(m_seed, _MM_SHUFFLE(2, 3, 0, 1));
    
    m_seed = _mm_mul_epu32(m_seed, multiplier);
    multiplier = _mm_shuffle_epi32(multiplier, _MM_SHUFFLE(2, 3, 0, 1));
    seed_split = _mm_mul_epu32(seed_split, multiplier);
    
    m_seed = _mm_and_si128(m_seed, mod_mask);
    seed_split = _mm_and_si128(seed_split, mod_mask);
    seed_split = _mm_shuffle_epi32(seed_split, _MM_SHUFFLE(2, 3, 0, 1));
    m_seed = _mm_or_si128(m_seed, seed_split);
    m_seed = _mm_add_epi32(m_seed, adder);
    
    return m_seed;
}

bool block_eq(block a, block b) {
    return memcmp(&a, &b, 16) == 0;
}

block new_tweak(int id)
{
    return _mm_set_epi64x((int64_t)id, (int64_t)id);
}

string block2hex(block a) {
    char buf[100];
    int cx;
    int64_t* v64val = (int64_t*)&a;
    cx = snprintf(buf, 100, "%.16llx %.16llx\n", (uint64_t)v64val[1], (uint64_t)v64val[0]);
    if (cx > 0 && cx < 100) {
        
        return string(buf);
    }
    perror("Buffer overflow for block2hex()");
    abort();
}

block new_tweak(int id);

int byte2label(char* src, int offset, block& lbl) {
    try {
        lbl = _mm_load_si128((__m128i*)(src + offset));
    } catch (const std::exception& e) {
        perror(e.what());
        abort();
    }
    return 0;
}

int select2bytes(vector<int> select_vec, char* dest) {
    int val;
    for (int i = 0; i < select_vec.size(); ++i) {
        val = select_vec[i];
        if (val != 0 && val != 1) {
            perror("Invalid selection");
            abort();
        }
        try {
            if (val == 1) {
                set_bit(dest[i / 8], i % 8);
            }
        } catch (const std::exception e) {
            perror(e.what());
            abort();
        }
    }
    return 0;
}

int label_vec_marshal(vector<block>& lblvec, char* dest) {
    block lbl;
    for (int i = 0; i < lblvec.size(); ++i) {
        lbl = lblvec[i];
        try {
            memcpy(dest + i * LABELSIZE, (char*)&lbl, LABELSIZE);
        } catch (const std::exception e) {
            perror(e.what());
            abort();
        }
    }
    return 0;
}

string find_n_replace(string s, string pattern, string subst)
{
    boost::replace_all(s, pattern, subst);
    return s;
}
