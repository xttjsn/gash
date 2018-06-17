//
//  garbled_circuit.hpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/17/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef garbled_circuit_hpp
#define garbled_circuit_hpp

#include <stdio.h>
#include "circuit.hpp"
#include "aes.hpp"
#include "util.hpp"

class GarbledCircuit;
class GarbledGate;
class GarbledWire;
class GarbledWireInstance;

typedef GarbledWireInstance GWI;
typedef GarbledWire GW;
typedef GarbledGate GG;
typedef GarbledCircuit GC;
typedef map<int, GWI*> IdGWIMap;
typedef map<int, GG*>  IdGGMap;

class GarbledWire : public Wire {
public:
    int                    m_id;
    block                  m_lbl0;               // Used for garbling
    block                  m_lbl1;
    block                  m_lbl;                // Used for evaluating

    GarbledWire (Wire* w) : m_id(w->m_id) {}
    GarbledWire (int id) : m_id(id) {}
    ~GarbledWire() {}
};

class GarbledWireInstance : public WI {
public:
    GW*                                 m_gw = NULL;
    bool                               m_inv = false;

    GarbledWireInstance(WI* wi) : WI(wi->m_wire, wi->m_inv), m_gw(new GW(wi->m_wire)), m_inv(wi->m_inv) {}
    
    inline int get_id() {
        return m_gw->m_id;
    }
    
    inline block get_lbl0() {
        return m_inv ? m_gw->m_lbl1: m_gw->m_lbl0;
    }
    
    inline block get_orig_lbl0() {
        return m_gw->m_lbl0;
    }
    
    inline block get_lbl1() {
        return m_inv ? m_gw->m_lbl0: m_gw->m_lbl1;
    }
    
    inline block get_orig_lbl1() {
        return m_gw->m_lbl1;
    }

    inline block get_lbl() {
        return m_gw->m_lbl;
    }
    
    inline void set_lbl0(block lbl0) {
        if (m_inv) {
            m_gw->m_lbl1 = lbl0;
        } else {
            m_gw->m_lbl0 = lbl0;
        }
    }
    
    inline void set_orig_lbl0(block lbl) {
        m_gw->m_lbl0 = lbl;
    }
    
    inline void set_orig_lbl1(block lbl) {
        m_gw->m_lbl1 = lbl;
    }
    
    inline void set_lbl1(block lbl1) {
        if (m_inv) {
            m_gw->m_lbl0 = lbl1;
        } else {
            m_gw->m_lbl1 = lbl1;
        }
    }
    
    inline void set_lbl(block lbl) {
        m_gw->m_lbl = lbl;
    }

    inline void set_id(int id) {
        m_gw->m_id = id;
    }
    
    void garble(block R) {
        block lbl0 = random_block();
        block lbl1 = xor_block(lbl0, R);
        set_lbl0(lbl0);
        set_lbl1(lbl1);
    }
    
    int get_smtc_w_lsb(int lsb) {
        if (lsb != 0 && lsb != 1) {
            perror("Invalid lsb, can only be 0 or 1");
            abort();
        }
        int lbl0lsb = get_lsb(get_lbl0());
        int lbl1lsb = get_lsb(get_lbl1());
        
        if (lbl0lsb == lsb) {
            return m_inv ? 1 : 0;
        } else if (lbl1lsb == lsb) {
            return m_inv ? 0 : 1;
        } else {
            perror("lsb is neither 0 or 1");
            abort();
        }
    }
    
    block get_lbl_w_smtc(int smtc) {
        if (smtc == 0) {
            return get_lbl0();
        } else {
            return get_lbl1();
        }
    }
    
    block get_orig_lbl_w_smtc(int smtc) {
        if (smtc == 0) {
            return get_orig_lbl0();
        } else {
            return get_orig_lbl1();
        }
    }
    
    block get_lbl_w_lsb(int lsb) {
        if (lsb != 0 && lsb != 1) {
            perror("Invalid lsb, can only be 0 or 1");
            abort();
        }
        
        int lbl0lsb = get_lsb(get_lbl0());
        int lbl1lsb = get_lsb(get_lbl1());
        
        if (lbl0lsb == lsb) {
            return get_lbl0();
        } else if (lbl1lsb == lsb) {
            return get_lbl1();
        } else {
            perror("Cannot find label with lsb, garbling should make sure that lsb are different");
            abort();
        }
    }
    
    void set_lbl_w_smtc(block lbl, int smtc) {
        if (smtc != 0 && smtc != 1) {
            perror("Invalid semantic.");
            abort();
        }
        
        if (smtc == 0) {
            set_lbl0(lbl);
        } else {
            set_lbl1(lbl);
        }
    }

    int recover_smtc() {
        if (block_eq(m_gw->m_lbl, get_lbl0())) {
            // return 0 ^ get_inv();
            return 0;
            
        } else if (block_eq(m_gw->m_lbl, get_lbl1())) {
            // return 1 ^ get_inv();
            return 1;
        } else {
            perror("Invalid label, neither label0 nor label 1");
            abort();
        }
    }
    
    ~GarbledWireInstance() {
        delete m_gw;
    }
};

class GarbledGate : public Gate {
public:
    bool m_is_xor;
    GWI *m_in0;
    GWI *m_in1;
    GWI *m_out;
    block m_egtt[4];
    
    GarbledGate() {}
    
    GarbledGate(bool is_xor, GWI* in0, GWI* in1, GWI* out):
    m_is_xor(is_xor),  m_in0(in0),  m_in1(in1),  m_out(out) {
    }
    
    GarbledGate(bool is_xor, GWI* in0, GWI* in1, GWI* out, block egtt[4]): m_is_xor(is_xor), m_in0(in0), m_in1(in1), m_out(out) {
        memcpy(m_egtt, egtt, 4 * sizeof(block));
    }

    ~GarbledGate() {}
};

class GarbledCircuit : public Circuit {
public:
    IdGWIMap                    m_gwi_map;
    IdGGMap                     m_gg_map;
    block                       m_R;
    
    GWI* get_gwi(int id) {
        auto it = m_gwi_map.find(id);
        if (it != m_gwi_map.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }
    
    GG* get_gg(int id)
    {
        auto it = m_gg_map.find(id);
        if (it != m_gg_map.end()) {
            return it->second;
        }
        return nullptr;
    }
    
    bool has_gg(int id) {
        return m_gg_map.find(id) != m_gg_map.end();
    }
    
    void add_gg(GG* gate) {
        if (has_gg(gate->get_id())) {
            perror("Gate already exists");
            abort();
        }
        m_gg_map.emplace(gate->get_id(), gate);
    }
    
    void add_gwi(GWI* gwi) {
        if (has_gwi(gwi->get_id())) {
            perror("GWI already exists");
            abort();
        }
        m_gwi_map.emplace(gwi->get_id(), gwi);
    }
    
    bool has_gwi(int id) {
        return m_gwi_map.find(id) != m_gwi_map.end();
    }
    
    int set_gwl(int id, block label) {
        if (!has_gwi(id)) {
            perror("GWI does not exists");
            abort();
        }
        m_gwi_map.find(id)->second->set_lbl(label);
        return 0;
    }
    
    void get_lbl(int id, int val, block& lbl) {
        assert(val == 0 || val == 1);
        auto it = m_gwi_map.find(id);
        if (it != m_gwi_map.end()) {
            lbl = it->second->get_lbl_w_smtc(val);
        }
    }
    
    int get_orig_lbl(int id, int val, block& lbl);
    
    void init() {
        srand_sse((int)time(0));
        m_R = random_block();
        set_lsb(m_R);
    }
    
    void garble() {
        block egtt[4];
        memset(egtt, 0, 4 * sizeof(block));
        
        block lbl0;
        block lbl1;
        block lbl;
        
        WI* wi;
        GWI* gwi;
        
        Gate* g;
        GG* gg;
        
        WI* in0;
        WI* in1;
        WI* out;
        
        GWI* gin0;
        GWI* gin1;
        GWI* gout;
        
        int func;
        int first_row_smtc;
        block ZERO = _mm_setzero_si128();
        block tweak;
        
        init();
        
        for (auto it = m_in_id_set.begin(); it != m_in_id_set.end(); ++it) {
            
            wi = get_wi(*it);
            gwi = new GWI(wi);
            gwi->garble(m_R);
            add_gwi(gwi);
        }
        
        for (auto it = m_gate_map.begin(); it != m_gate_map.end(); ++it) {
            
            g = it->second;
            in0 = g->m_in0;
            in1 = g->m_in1;
            out = g->m_out;
            func = g->m_func;
            
            gin0 = get_gwi(in0->m_wire->m_id);
            gin1 = get_gwi(in1->m_wire->m_id);
            gout = new GWI(out);
            
            /**
             * Garbling an XOR gate
             */
            if (func == funcXOR) {
                
                lbl0 = xor_block(gin0->get_lbl0(), gin1->get_lbl0());
                gout->set_lbl0(lbl0);
                
                lbl1 = xor_block(lbl0, m_R);
                gout->set_lbl1(lbl1);
                add_gwi(gout);
                
                gg = new GG(1, gin0, gin1, gout);
                
            } else {
                
                /**
                 * Garbling an AND/OR gate
                 *
                 */
                
                // Construct the tweak
                tweak = new_tweak(g->get_id());
                
                // Get the semantic of the first row of egtt
                first_row_smtc = eval(func, gin0->get_smtc_w_lsb(0), gin1->get_smtc_w_lsb(0));
                
                // Encrypt the label
                lbl = encrypt(gin0->get_lbl_w_lsb(0), gin1->get_lbl_w_lsb(0), tweak, ZERO,
                              AESkey);
                
                gout->set_lbl_w_smtc(lbl, first_row_smtc);
                gout->set_lbl_w_smtc(xor_block(lbl, m_R), first_row_smtc ^ 1);
                
                lbl0 = gout->get_lbl_w_smtc(0);
                lbl1 = gout->get_lbl_w_smtc(1);
                
                // Add output wire to circuit
                add_gwi(gout);
                
                // Write label to encrypted garbled truth table
                egtt[1] = encrypt(gin0->get_lbl_w_lsb(1),
                                 gin1->get_lbl_w_lsb(0),
                                 tweak,
                                 gout->get_lbl_w_smtc(eval(func, gin0->get_smtc_w_lsb(1), gin1->get_smtc_w_lsb(0))),
                                 AESkey);
                
                egtt[2] = encrypt(gin0->get_lbl_w_lsb(0),
                                 gin1->get_lbl_w_lsb(1),
                                 tweak,
                                 gout->get_lbl_w_smtc(eval(func, gin0->get_smtc_w_lsb(0), gin1->get_smtc_w_lsb(1))),
                                 AESkey);
                
                egtt[3] = encrypt(gin0->get_lbl_w_lsb(1),
                                 gin1->get_lbl_w_lsb(1),
                                 tweak,
                                 gout->get_lbl_w_smtc(eval(func, gin0->get_smtc_w_lsb(1), gin1->get_smtc_w_lsb(1))),
                                 AESkey);
                
                // Create the garbled gate with garbled truth table
                gg = new GG(0, gin0, gin1, gout, egtt);
            }
            
            // Add garbled gate to the garbled circuit
            add_gg(gg);
        }
    }
    
    void evaluate() {
        block tweak;
        block lbl;
        block ZERO = _mm_setzero_si128();
        
        GWI* gin0;
        GWI* gin1;
        GWI* gout;
        GG* gg;
        
        int id;
        int select;
        
        for (auto it = m_gg_map.begin(); it != m_gg_map.end(); ++it) {
            
            id = it->first;
            gg = get_gg(id);
            
            if (!gg) {
                perror("Cannot find garbled gate");
                abort();
            }
            
            gin0 = gg->m_in0;
            gin1 = gg->m_in1;
            gout = gg->m_out;
            
            // XOR gate
            if (gg->m_is_xor) {
                
                lbl = xor_block(gin0->get_lbl(), gin1->get_lbl());
                
            } else {
                // Non-XOR gate
                
                select = get_lsb(gin0->get_lbl()) + (get_lsb(gin1->get_lbl()) << 1);
                tweak = new_tweak(gg->get_id());
                
                if (select == 0) {
                    
                    // Use the same encryption as in garbling to get the label
                    lbl = encrypt(gin0->get_lbl(), gin1->get_lbl(), tweak, ZERO, AESkey);
                    
                } else {
                    
                    lbl = decrypt(gin0->get_lbl(),
                                  gin1->get_lbl(),
                                  tweak,
                                  gg->m_egtt[select],
                                  AESkey);
                }
            }
            
            gout->set_lbl(lbl);
        }
    }
    
    GarbledCircuit(){}
    ~GarbledCircuit(){}

};

#endif /* garbled_circuit_hpp */
