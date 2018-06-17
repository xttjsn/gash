//
//  gash_swift_api.cpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/16/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#include "gash_swift_api.hpp"
#include "circuit.hpp"
#include "div_circuit.hpp"
#include "garbled_circuit.hpp"
#include "garbler.hpp"

CCircuit* CreateCircuit() {
    return new CCircuit();
}

CDivCircuit* CreateCDivCircuit(int bitsize, int denom, int nume) {
    return new CDivCircuit(bitsize, denom, nume);
}

void BuildDivCircuit(CDivCircuit* circ) {
    circ->build();
}

void ExecuteCircuit(CCircuit* circ) {
    circ->execute();
}

const char* GetOutputString(CCircuit* circ) {
    char* out_chars = new char[2048];
    memset(out_chars, 0, 2048);
    string out_str;
    int len = (int)circ->m_out_id_vec.size();
    for (int i = 0; i < len; ++i) {
        WI* wi = circ->get_wi(circ->m_out_id_vec[i]);
        int v = wi->m_inv ? wi->m_wire->m_val ^ 1 : wi->m_wire->m_val;
        out_str += (v == 0 ? "0" : "1");
    }
    std::reverse(out_str.begin(), out_str.end());
    memcpy(out_chars, out_str.c_str(), out_str.size());
    return out_chars;
}

