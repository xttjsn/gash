//
//  circuit.hpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/16/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef circuit_hpp
#define circuit_hpp

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <gmpxx.h>
#include <algorithm>
#include "util.hpp"
#include "common.hpp"

/* Arithmetic operations */
#define AOP_PLUS 0x00
#define AOP_SUB 0x01
#define AOP_UMINUS 0x02
#define AOP_MUL 0x03
#define AOP_DIV 0x04

/// If advanced arithmetic is enabled
#ifdef __ADV_ARITH__

#define AOP_SQR 0x05
#define AOP_SQRT 0x06

/// If piece wise linear approximation is enabled
#ifdef __PWS_LIN_APPRX__

#define AOP_LOG2 0x07
#define AOP_LOG10 0x08

#endif
#endif

/* Bitwise operations */
#define BOP_OR 0x10
#define BOP_AND 0x11
#define BOP_XOR 0x12
#define BOP_INV 0x13
#define BOP_SHL 0x14
#define BOP_SHR 0x15

/* Comparison operations */
#define COP_LA 0x20 // Larger than
#define COP_LE 0x21 // Less than
#define COP_LAE 0x22 // Larger than or equal to
#define COP_LEE 0x23 // Less or equal to
#define COP_EQ 0x24 // Equal
#define COP_NEQ 0x25 // Not equal

/* Gate type */
#define opIAND 1
#define opAND 8
#define opOR 14
#define opXOR 6
#define opDFF 17

using std::vector;
using std::map;
using std::set;
using std::string;

class Wire;
class WireInstance;
class Gate;
class Circuit;
typedef WireInstance WI;



/**
 * Typedef a bunch of helper classes
 *
 */
typedef map<int, int> IdValMap;
typedef map<int, int> IdIdMap;
typedef map<int, WI*> IdWiMap;
typedef map<int, Gate*> IdGateMap;
typedef vector<int>   IdVec;
typedef set<int>      IdSet;
typedef vector<Gate*> GateList;

enum GateFunc {
    funcAND = 8,
    funcXOR = 6,
    funcOR  = 14,
    funcIAND = 1
};

class Wire {
public:
    int m_id = -1;
    int m_val = -1;
    
    Wire() {}
    Wire(int id) : m_id(id) {}
    Wire(int id, int v) : m_id(id), m_val(v) {}
};

class WireInstance {
public:
    Wire* m_wire = nullptr;
    bool m_inv = false;
    int m_inv_count = 0;
    
    WireInstance(Wire* wire, bool inv): m_wire(wire), m_inv(inv) {}
    WireInstance(int id, bool inv): m_wire(new Wire(id)), m_inv(inv) {}
    void invert() {
        if (m_inv_count != 0) {
            perror("Wire is inverted twice!");
            abort();
        }
        m_inv = !m_inv;
        m_inv_count++;
    }
};

class Bundle {
public:
    vector<WI*> m_wis;
    /// true if this bundle is derived from a constant value.
    /// Default false.
    /// If true, then during assignment, it is allowed to use only part
    /// of the bundle
    bool          m_isconst = false;
    
    Bundle() {}
    
    Bundle(vector<WI*>& wires) : m_wis(wires) {}
    Bundle(WI* w) { m_wis.push_back(w); }
    void push_back(WI* w) { m_wis.push_back(w); }
    WI*& operator[](int i) { return m_wis[i]; }
    void push_front(WI* w) { m_wis.insert(m_wis.begin(), w); }
    WI* back() { return m_wis.back(); }
    int size() { return m_wis.size(); }
    int copyfrom(Bundle& src, int start, int src_start, int size) {
        if (size > this->size() - start) {
            perror("This bundle has not enough room to copy from another bundle.");
            abort();
        }
        if (src.size() - src_start < size) {
            perror("The source bundle has not enough wires.");
            abort();
        }
        for (int i = 0; i < size; ++i) {
            m_wis[i + start] = src[i + src_start];
        }
        return 0;
    }
    void clear() { m_wis.clear(); }

};

class Gate {
public:
    int m_func;
    WI* m_in0;
    WI* m_in1;
    WI* m_out;
    
    Gate() {}
    
    Gate(int func, WI* in0, WI* in1, WI* out)
    : m_out(out), m_func(func) , m_in0(in0), m_in1(in1) {}
    
    inline int get_id() {
        return m_out->m_wire->m_id;
    }
};

class Circuit {
public:
    Bundle        m_in;
    Bundle        m_out;
    
    IdWiMap       m_wi_map;
    IdGateMap     m_gate_map;
    IdIdMap       m_wi_inv_map;
    IdSet         m_in_id_set;
    IdVec         m_out_id_vec;
    
    WI*           m_wi_one;
    WI*           m_wi_zero;
    
    int           m_wcount;
    
    Circuit() {
        m_wi_one = nextwi();
        m_wi_one->m_wire->m_val = 1;
        m_wi_zero = nextwi();
        m_wi_zero->m_wire->m_val = 0;
    }
    
    void clear() {
        m_in.clear();
        m_out.clear();
        m_wi_map.clear();
        m_gate_map.clear();
        m_wi_inv_map.clear();
        m_in_id_set.clear();
        m_out_id_vec.clear();
        m_wcount = 0;
        delete m_wi_one;
        delete m_wi_zero;
        m_wi_one = nextwi();
        m_wi_one->m_wire->m_val = 1;
        m_wi_zero = nextwi();
        m_wi_zero->m_wire->m_val = 0;
    }
    
    inline WI* get_wi(int id) {
        if (m_wi_map.find(id) == m_wi_map.end())
            return nullptr;
        else
            return m_wi_map.find(id)->second;
    }
    
    inline Gate* get_gate(int id) {
        if (m_gate_map.find(id) == m_gate_map.end())
            return nullptr;
        else
            return m_gate_map.find(id)->second;
    }
    
    inline void add_wi(WI* wi) {
        if (m_wi_map.find(wi->m_wire->m_id) != m_wi_map.end()) {
            perror("Duplicate wire addition");
            abort();
        }
        m_wi_map.emplace(wi->m_wire->m_id, wi);
    }
    
    inline void add_gate(Gate* g) {
        if (m_gate_map.find(g->get_id()) != m_gate_map.end()) {
            perror("Duplicate gate addition");
            abort();
        }
        m_gate_map.emplace(g->get_id(), g);
    }
    
    inline bool has_gate(int id) {
        return m_gate_map.find(id) != m_gate_map.end();
    }
    
    inline void create_gate(int funct, int outid, int in0id, int in1id, bool in0inv, bool in1inv) {
        if (has_gate(outid)) {
            perror("Gate already exists");
            abort();
        }
        
        WI* out = get_wi(outid);
        WI* in0 = get_wi(in0id);
        WI* in1 = get_wi(in1id);
        
        if (out == nullptr) {
            out = new WI(outid, false);
            add_wi(out);
        }
        
        if (in0 == nullptr || in1 == nullptr) {
            abort();
        }
        
        if (in0->m_inv != in0inv) {
            in0->invert();
        }
        
        if (in1->m_inv != in1inv) {
            in1->invert();
        }
        
        Gate* g = new Gate(funct, out, in0, in1);
        add_gate(g);
    }
    
    inline void write_gate(GateFunc func, WI* a, WI* b, WI* c) {
        Gate* g = new Gate(func, a, b, c);
        add_gate(g);
    }
    
    inline WI* nextwi() {
        m_wcount++;
        WI* wi = new WI(m_wcount, false);
        add_wi(wi);
        return wi;
    }
    
    inline WI* zerowi() {
        return m_wi_zero;
    }
    
    inline WI* onewi() {
        return m_wi_one;
    }
    
    void execute() {
        for (auto it = m_gate_map.begin(); it != m_gate_map.end(); it++) {
            Gate* g = it->second;
            WI* in0 = g->m_in0;
            WI* in1 = g->m_in1;
            WI* out = g->m_out;
            int funct = g->m_func;
            if (in0->m_wire->m_val < 0 || in1->m_wire->m_val < 0) {
                perror("Wire has negative value");
                abort();
            }
            out->m_wire->m_val = eval(funct,
                                      in0->m_inv ? in0->m_wire->m_val ^ 1 :
                                                    in0->m_wire->m_val,
                                      in1->m_inv ? in1->m_wire->m_val ^ 1 :
                                                    in1->m_wire->m_val
                                      );
//            printf("out wire %d val:  %d\n", out->m_wire->m_id, out->m_wire->m_val);
        }
    }
    string get_output_str();
    
    /************** Evaluation functions ************/
    WI* evalw_XOR(WI* a, WI* b) {
        WI* w = nextwi();
        write_gate(funcXOR, a, b, w);
        return w;
    }
    
    WI* evalw_AND(WI* a, WI* b) {
        WI* w = nextwi();
        write_gate(funcAND, a, b, w);
        return w;
    }
    
    WI* evalw_OR(WI* a, WI* b) {
        WI* w = nextwi();
        write_gate(funcOR, a, b, w);
        return w;
    }
    
    WI* evalw_IAND(WI* a, WI* b) {
        WI* w = nextwi();
        write_gate(funcIAND, a, b, w);
        return w;
    }
    
    WI* evalw_INV(WI* a) {
        if (m_wi_inv_map.find(a->m_wire->m_id) != m_wi_inv_map.end()) {
            return get_wi(m_wi_inv_map.find(a->m_wire->m_id)->second);
        }
        
        if (m_in_id_set.find(a->m_wire->m_id) != m_in_id_set.end()) {
            // Input wire
            WI* w_inv = nextwi();
            w_inv->m_wire->m_val = 1 ^ a->m_wire->m_val;
            m_wi_inv_map.emplace(w_inv->m_wire->m_id, a->m_wire->m_id);
            m_wi_inv_map.emplace(a->m_wire->m_id, w_inv->m_wire->m_id);
            m_in_id_set.insert(w_inv->m_wire->m_id);
            return w_inv;
        }
        else {
            Gate* g = get_gate(a->m_wire->m_id);
            if (g != nullptr) {
                // Non constant wire
                WI* new_out = nextwi();
                new_out->m_wire->m_val = a->m_wire->m_val;
                new_out->m_inv = !a->m_inv;
                Gate* new_g = new Gate(g->m_func, g->m_in0, g->m_in1, new_out);
                add_gate(new_g);
                
                m_wi_inv_map.emplace(new_out->m_wire->m_id, a->m_wire->m_id);
                m_wi_inv_map.emplace(a->m_wire->m_id, new_out->m_wire->m_id);
                return new_out;
            } else {
                // Constant wire
                if (a->m_wire->m_val == 0) {
                    return onewi();
                } else {
                    return zerowi();
                }
            }
        }
    }
    
    WI* evalw_FADD(WI* a, WI* b, WI*& cin) {
        WI* w_xor = evalw_XOR(a, b);
        WI* w_sum = evalw_XOR(w_xor, cin);
        WI* w_and1 = evalw_AND(w_xor, cin);
        WI* w_and2 = evalw_AND(a, b);
        WI* w_newcin = evalw_OR(w_and1, w_and2);
        cin = w_newcin;
        return w_sum;
    }
    
    Bundle* evala_ADD(Bundle& a, Bundle& b) {
        assert(a.size() == b.size());
        WI* w_cin = zerowi();
        Bundle* b_out = new Bundle();
        for (int i = 0; i < a.size(); ++i) {
            WI* w_a = a[i];
            WI* w_b = b[i];
            WI* w_sum = evalw_FADD(w_a, w_b, w_cin);
            b_out->push_back(w_sum);
        }
        return b_out;
    }
    
    Bundle* evala_ADD_raw(Bundle& a, Bundle& b, WI*& w_cin) {
        assert(a.size() == b.size());
        w_cin = zerowi();
        Bundle* b_out = new Bundle();
        for (int i = 0; i < a.size(); ++i) {
            WI* w_a = a[i];
            WI* w_b = b[i];
            WI* w_sum = evalw_FADD(w_a, w_b, w_cin);
            b_out->push_back(w_sum);
        }
        return b_out;
    }
    
    Bundle* evala_UMINUS(Bundle& a) {
        Bundle tmp0;
        Bundle tmp1;
        for (int i = 0; i < a.size(); ++i) {
            WI* w_inv = evalw_INV(a[i]);
            tmp0.push_back(w_inv);
        }
        
        tmp1.push_back(onewi());
        for (int i = 1; i < a.size(); ++i) {
            tmp1.push_back(zerowi());
        }
        
        return evala_ADD(tmp0, tmp1);
    }
    
    Bundle* evala_SUB(Bundle& a, Bundle& b) {
        Bundle* b_neg = evala_UMINUS(b);
        Bundle* b_out = evala_ADD(a, *b_neg);
        delete b_neg;
        return b_out;
    }
    
    Bundle* evala_MUL(Bundle& a, Bundle& b) {
        Bundle *prev_res;
        Bundle res;
        Bundle* out;
        Bundle tmp0;
        Bundle tmp1;
        
        WI* cin;
        int len = a.size();
        for (int i = 0; i < len; ++i) {
            // One level of multiplication
            for (int j = 0; j < len; ++j) {
                WI* w = nextwi();
                write_gate(funcAND, a[i], b[j], w);
                res.push_back(w);
            }
            
            // If this is not the first level
            if (prev_res->size() > 0) {
                for (int k = 0; k < i; ++k) {
                    tmp0.push_back(zerowi());
                }
                for (int k = 0; k < res.size(); ++k) {
                    tmp0.push_back(res[k]);
                }
                
                prev_res = evala_ADD_raw(*prev_res, tmp0, cin);
                (*prev_res)[prev_res->size() - 2] = cin;
                
            } else {
                *prev_res = res;
                (*prev_res)[prev_res->size() - 2] = zerowi();
            }
            res.clear();
            tmp0.clear();
            tmp1.clear();
        }
        
        out = new Bundle(*prev_res);
        return out;
    }
    
    Bundle* evala_DVG(Bundle& a, Bundle& b, WI*& r) {
        assert(a.size() == b.size());
        WI* a_LAE_b = evalc_LAE(a, b);
        WI* a_LE_b = evalw_INV(a_LAE_b);
        Bundle* sub = evala_SUB(a, b);
        
        Bundle* out = new Bundle();
        for (int i = 0; i < a.size(); ++i) {
            WI* w = evalw_AND(a_LAE_b, (*sub)[i]);
            WI* w_orig = evalw_AND(a_LE_b, a[i]);
            WI* w_final = evalw_OR(w, w_orig);
            out->push_back(w_final);
        }
        
        r = a_LAE_b;
        delete sub;
        return out;
    }
    
    Bundle* evala_DIV(Bundle& a, Bundle& b) {
        assert(a.size() == b.size());
        Bundle* out = new Bundle();
        int len = (int)a.size();
        WI* w_sign_different = evalw_XOR(a.back(), b.back());
        WI* w_sign_same      = evalw_INV(w_sign_different);
        WI* w_a_neg          = a.back();
        WI* w_b_neg          = b.back();
        WI* w_a_pos          = evalw_INV(w_a_neg);
        WI* w_b_pos          = evalw_INV(w_b_neg);
        
        Bundle* a_inv = evala_UMINUS(a);
        Bundle* b_inv = evala_UMINUS(b);
        
        Bundle* active_a = new Bundle();
        for (int i = 0; i < len; ++i) {
            WI* w = evalw_AND(a[i], w_a_pos);
            WI* w_inv = evalw_AND((*a_inv)[i], w_a_neg);
            WI* w_final = evalw_OR(w, w_inv);
            active_a->push_back(w_final);
        }
        
        Bundle* active_b = new Bundle();
        for (int i = 0; i < len; ++i) {
            WI* w = evalw_AND(b[i], w_b_pos);
            WI* w_inv = evalw_AND((*b_inv)[i], w_b_neg);
            WI* w_final = evalw_OR(w, w_inv);
            active_b->push_back(w_final);
        }
        
        Bundle* sub_res = new Bundle();
        for (int i = 1; i < len; ++i) {
            sub_res->push_back(zerowi());
        }
        
        Bundle* tmp = new Bundle();
        for (int i = 0; i < len; ++i) {
            tmp->push_back((*active_a)[len - i - 1]);
            for (int j = 0; j < len - 1; ++j) {
                tmp->push_back((*sub_res)[j]);
            }
            
            delete sub_res;
            WI* r;
            sub_res = evala_DVG(*tmp, *active_b, r);
            out->push_front(r);
            tmp->clear();
        }
        
        // Change sign back
        Bundle* out_inv = evala_UMINUS(*out);
        for (int i = 0; i < out->size(); ++i) {
            WI* w = evalw_AND((*out)[i], w_sign_same);
            WI* w_inv = evalw_AND((*out_inv)[i], w_sign_different);
            WI* w_final = evalw_OR(w, w_inv);
            (*out)[i] = w_final;
        }
        
        delete active_a;
        delete active_b;
        delete sub_res;
        delete tmp;
        return out;
        
    }
    
    Bundle* evalb_AND(Bundle& a, Bundle& b) {
        GASSERT(a.size() == b.size());
        int len = a.size();
        Bundle* out = new Bundle();
        WI* w;
        
        // Evaluate AND for each bit
        for (int i = 0; i < len; ++i) {
            w = evalw_AND(a[i], b[i]);
            out->push_back(w);
        }
        return out;
    }
    
    Bundle* evalb_OR(Bundle& a, Bundle& b) {
        GASSERT(a.size() == b.size());
        int len = a.size();
        Bundle* out = new Bundle();
        WI* w;
        // Evaluate OR for each bit
        for (int i = 0; i < len; ++i) {
            w = evalw_OR(a[i], b[i]);
            out->push_back(w);
        }
        return out;
    }
    
    Bundle* evalb_XOR(Bundle& a, Bundle& b) {
        GASSERT(a.size() == b.size());
        int len = a.size();
        Bundle* out = new Bundle();
        WI* w;
        // Evaluate XOR for each bit
        for (int i = 0; i < len; ++i) {
            w = evalw_XOR(a[i], b[i]);
            out->push_back(w);
        }
        return out;
    }
    
    Bundle* evalb_INV(Bundle& a) {
        int len = (int)a.size();
        Bundle* out = new Bundle();
        WI* w;
        // Evaluate INV for each bit
        for (int i = 0; i < len; ++i) {
            w = evalw_INV(a[i]);
            out->push_back(w);
        }
        return out;
    }
    
    Bundle* evalb_SHL(Bundle& a, int n) {
        Bundle* out = new Bundle();
        // First, fill min(n, in.size()) zero wries to `out`
        for (int i = 0; i < std::min(n, (int)a.size()); ++i) {
            out->push_back(zerowi());
        }
        // Then, copy in.size() - n wires to out
        for (int i = 0; i < a.size() - n; ++i) {
            out->push_back(a[i]);
        }
        
        return out;
    }
    
    Bundle* evalb_SHR(Bundle& a, int n) {
        Bundle* out = new Bundle();
        // First, copy in.size() - n wires to `out`
        for (int i = 0; i < a.size() - n; ++i) {
            out->push_back(a[i]);
        }
        
        // Then, fill min(n, in.size()) zero wires to `out`
        for (int i = 0; i < std::min(n, (int)a.size()); ++i) {
            out->push_back(zerowi());
        }
        
        return out;
    }
    
    
    WI* evalc_LA(Bundle& a, Bundle& b) {
        assert(a.size() == b.size());
        assert(a.size() > 1);
        
        Bundle xors;
        for (int i = 0; i < a.size(); ++i) {
            WI* w = evalw_XOR(a[i], b[i]);
            WI* w_inv = evalw_INV(w);
            xors.push_back(w_inv);
        }
        
        WI* w_inv_a = evalw_INV(a.back());
        WI* w_ret = evalw_AND(w_inv_a, b.back());
        
        WI* w_used_xor = onewi();
        WI* w_iand_a_b = evalw_IAND(a.back(), b.back());
        WI* w_and_a_b  = evalw_AND(a.back(), b.back());
        
        for (int i = 1; i < a.size(); ++i) {
            WI* w_inv_b = evalw_INV(b[a.size() - i - 1]);
            WI* w_and_a_ib = evalw_AND(a[a.size() - i - 1], w_inv_b);
            WI* w_and_a_ib_xor = evalw_AND(w_and_a_ib, w_used_xor);
            WI* w_irow = evalw_AND(w_and_a_ib_xor, w_iand_a_b);
            w_ret = evalw_OR(w_ret, w_irow);
            
            WI* w_inv_a = evalw_INV(a[a.size() - i - 1]);
            WI* w_and_ia_b = evalw_AND(b[a.size() - i - 1], w_inv_a);
            WI* w_and_ia_b_xor = evalw_AND(w_and_ia_b, w_used_xor);
            WI* w_jrow = evalw_AND(w_and_ia_b_xor, w_and_a_b);
            w_ret = evalw_OR(w_ret, w_jrow);
            
            w_used_xor = evalw_AND(w_used_xor, xors[a.size() - i - 1]);
        }
        return w_ret;
    }
    
    WI* evalc_LE(Bundle& a, Bundle& b) {
        return evalc_LA(b, a);
    }
    
    WI* evalc_EQ(Bundle& a, Bundle& b) {
        assert(a.size() == b.size());
        WI* w_ret = onewi();
        for (int i = 0; i < a.size(); ++i) {
            WI* w = evalw_XOR(a[i], b[i]);
            WI* w_inv = evalw_INV(w);
            w_ret = evalw_AND(w_inv, w_ret);
        }
        return w_ret;
    }
    
    WI* evalc_NEQ(Bundle& a, Bundle& b) {
        WI* w = evalc_EQ(a, b);
        return evalw_INV(w);
    }
    
    WI* evalc_LAE(Bundle& a, Bundle& b) {
        WI* w_la = evalc_LA(a, b);
        WI* w_eq = evalc_EQ(a, b);
        WI* w_lae = evalw_OR(w_la, w_eq);
        return w_lae;
    }
    
    WI* evalc_LEE(Bundle& a, Bundle& b) {
        WI* w_la = evalc_LA(b, a);
        WI* w_eq = evalc_EQ(b, a);
        WI* w_lae = evalw_OR(w_la, w_eq);
        return w_lae;
    }
    
    Bundle* evalo_if(WI* cond, Bundle& then_res, Bundle& else_res)
    {
        // out = (then_res & cond) | (else_res & cond')
        GASSERT(then_res.size() == else_res.size());
        int len = then_res.size();
        Bundle left;
        Bundle right;
        Bundle* out = new Bundle();
        WI* w;
        WI* cond_inv;
        
        for (int i = 0; i < len; ++i) {
            w = evalw_AND(cond, then_res[i]);
            left.push_back(w);
        }
        for (int i = 0; i < len; ++i) {
            cond_inv = evalw_INV(cond);
            w = evalw_AND(cond_inv, else_res[i]);
            right.push_back(w);
        }
        for (int i = 0; i < len; ++i) {
            w = evalw_OR(left[i], right[i]);
            out->push_back(w);
        }
        return out;
    }
    
    Bundle* num2bundle_n(int64_t v, int n)
    {
        int i = 0;
        Bundle* out = new Bundle();
        while (i < 64 && i < n) {
            out->push_back(getbit(v, i) == 1 ? onewi() : zerowi());
            i++;
        }
        out->m_isconst = true;
        return out;
    }
    
    
};



#endif /* circuit_hpp */
