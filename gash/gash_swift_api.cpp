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
#include "evaluator.hpp"
#include "func.hpp"

static Garbler m_garbler;
static Evaluator m_evaluator;
static string m_circ;
static string m_circ_name;
static string m_data_name;

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

const char* LoadCircuitFunc(const char* circ, int bitsize) {
    LOAD_CIRCUIT(string(circ), bitsize, m_circ);
    m_circ_name = string(circ) + "-" + std::to_string(bitsize);
    m_data_name = string(circ) + "-" + std::to_string(bitsize) + "data";
    return m_circ.c_str();
}

void SetCircuitFunc(const char* circ_func) {
    m_circ = string(circ_func);
}

void StartGarbler() {
    
//    m_garbler.init_connection();
//    ofstream m_circ_stream = ofstream(m_circ_name, std::ios::out | std::ios::trunc);
//    ofstream m_data_stream = ofstream(m_data_name, std::ios::out | std::ios::trunc);
//    extern FILE* yyin;
//    const char* src = m_circ;
//    yyin = std::tmpfile();
//    std::fputs(src, yyin);
//    std::rewind(yyin);
//    set_ofstream(m_circ_stream, m_data_stream);
//    EXPECT_EQ_with_Timer(0, yyparse(), "Parsing");
//    Garbler garbler(e_ip, port, ot_port, g_circ, g_dat);
//    EXPECT_EQ_with_Timer(0, garbler.build_circ(), "Build circuit");
//    EXPECT_EQ_with_Timer(0, garbler.read_input(), "Read input");
//    EXPECT_EQ_with_Timer(0, garbler.garble_circ(), "Garble circuit");
//    EXPECT_EQ_with_Timer(0, garbler.init_connection(), "Init connection");
//    EXPECT_EQ_with_Timer(0, garbler.send_egtt(), "Send encrypted garbled truth tables");
//    EXPECT_EQ_with_Timer(0, garbler.send_peer_lbls(), "Send peer labels");
//    EXPECT_EQ_with_Timer(0, garbler.send_self_lbls(), "Send self labels");
//    EXPECT_EQ_with_Timer(0, garbler.send_output_map(), "Send output map");
//    EXPECT_EQ_with_Timer(0, garbler.recv_output(), "Receive output");
//    EXPECT_EQ_with_Timer(0, garbler.report_output(), "Report output");
}

void StartEvaluator(const char* garbler_ip) {
}

const char* GetGarblerRawOutput() {
    return "000000001";
}
const char* GetEvaluatorRawOutput() {
    return "111110000";
}

const char* GetGarblerOutput() {
    return "13";
}

const char* GetEvaluatorOutput() {
    return "22";
}

