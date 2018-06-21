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
#include <boost/algorithm/string/replace.hpp>
#include "aes.hpp"

#define LABELSIZE 16
#define XORNUM 1997
#define NONXORNUM 2009
#define TimeIt(expr, event_name)                       \
    expr;
//    timer.tic(event_name);                             \
//    expr;                                              \
//    timer.toc()


using std::string;
using std::vector;
using std::map;

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

void srand_sse(int seed);
block random_block();
bool block_eq(block a, block b);
string block2hex(block a);
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
int byte2label(char* src, int offset, block& lbl);
int select2bytes(vector<int> select_vec, char* dest);
int label_vec_marshal(vector<block>& lblvec, char* dest);

string find_n_replace(string s, string pattern, string subst);

#endif /* util_hpp */
