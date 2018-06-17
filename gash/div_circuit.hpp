//
//  div_circuit.hpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/16/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef div_circuit_hpp
#define div_circuit_hpp

#include <stdio.h>
#include "circuit.hpp"

class DivCircuit : public Circuit {
public:
    int m_bitsize;
    int m_denom;
    int m_nume;
    
    DivCircuit(int bitsize, int denom, int nume)
    : m_bitsize(bitsize), m_denom(denom), m_nume(nume) {}
    
    void build() {
        Bundle* in0 = new Bundle();
        for (int i = 0; i < m_bitsize; ++i) {
            WI* wi = nextwi();
            wi->m_wire->m_val = getbit(m_denom, i);
            in0->push_back(wi);
            m_in_id_set.insert(wi->m_wire->m_id);
        }
        
        Bundle* in1 = new Bundle();
        for (int i = 0; i < m_bitsize; ++i) {
            WI* wi = nextwi();
            wi->m_wire->m_val = getbit(m_nume, i);
            in1->push_back(wi);
            m_in_id_set.insert(wi->m_wire->m_id);
        }
        
        Bundle* out = evala_DIV(*in0, *in1);
        for (auto it = out->begin(); it != out->end(); ++it) {
            m_out_id_vec.push_back((*it)->m_wire->m_id);
        }
    }
};


#endif /* div_circuit_hpp */
