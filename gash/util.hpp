//
//  util.hpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/17/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef util_hpp
#define util_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <gmpxx.h>
#include <vector>
#include <map>
#include "aes.hpp"

#define LABELSIZE 128
#define XORNUM 1997
#define NONXORNUM 2009

using std::string;
using std::vector;
using std::map;

static block m_seed;

class Event {
public:
    string m_name;
    long m_elapsed_time;
    Event(string name, long elapsed_time) : m_name(name), m_elapsed_time(elapsed_time) {}
};

typedef map<int, Event> EventMap;
class Timer {
    EventMap        m_records;
    int             m_nrecords = 0;
    bool            m_ticking = 0;
    long            m_elapsed_time = 0;
    timespec        m_last_time;
    timespec        m_curr_time;
    const char*     m_curr_name;
    
public:
    
    void tic(const char* event_name) {
        if (m_ticking) {
            toc();
            tic(event_name);
            
        } else {
            m_ticking = true;
            clock_gettime(CLOCK_MONOTONIC, &m_last_time);
            m_curr_name = event_name;
        }
    }
    
    void toc() {
        clock_gettime(CLOCK_MONOTONIC, &m_curr_time);
        m_elapsed_time = (m_curr_time.tv_sec - m_last_time.tv_sec) * 1000000000 + (m_curr_time.tv_nsec - m_last_time.tv_nsec);
        Event event(string(m_curr_name), m_elapsed_time);
        m_records.emplace(m_nrecords++, event);
        m_ticking = false;
    }
    
    void report() {
        for (const auto& it : m_records) {
            printf("%s, ", it.second.m_name.c_str());
        }
        printf("\n");
        
        for (const auto& it : m_records) {
            printf("%ld, ", it.second.m_elapsed_time);
        }
    }
    Timer(){}
};

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

inline int get_lsb(block label) {
    uint8_t *var = (uint8_t *) &label;
    return (*(var) & 1) == 1 ? 1 : 0;
}

inline void set_lsb(block& lbl) {
    uint8_t *p = (uint8_t*)&lbl;
    *p |= 1;
}

inline int getbit(int n, int i) {
    mpz_class mpz = n;
    return mpz_tstbit(mpz.get_mpz_t(), i);
}

inline int eval(int f, int a, int b) {
    return getbit(f, (a + (b << 1L)));
}

inline void set_bit(char& src, int idx) {
    src |= 1UL << idx;
}


inline block xor_block(block a, block b) {
    return _mm_xor_si128(a, b);
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




#endif /* util_hpp */
