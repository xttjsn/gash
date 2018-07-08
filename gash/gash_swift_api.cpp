//
//  gash_swift_api.cpp
//  CircuitExecutor
//
//  Created by Xiaoting Tang on 6/16/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#include <benchmark/benchmark.h>
#include "gash_swift_api.hpp"
#include "tcp.hpp"
#include "circuit.hpp"
#include "div_circuit.hpp"
#include "garbled_circuit.hpp"
#include "garbler.hpp"
#include "evaluator.hpp"
#include "func.hpp"
#include "util.hpp"
#include "lang.hpp"
//#include "seclenet.hpp"
//#include "seclenet_main.hpp"

static Garbler m_garbler;
static Evaluator m_evaluator;
static string m_circ;
static string m_circ_name;
static string m_data_name;
static Timer timer;

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

void StartGarbler(const char* evaluator_ip) {
    extern FILE* yyin;
    const char* src = m_circ.c_str();
    yyin = std::tmpfile();
    std::fputs(src, yyin);
    std::rewind(yyin);
    set_garbler(&m_garbler);
    m_garbler.m_peer_ip = string(evaluator_ip);
    TimeIt(yyparse(), "Parsing");
    m_garbler.check_ids();
    TimeIt(m_garbler.m_gc.garble(), "Garbling circuit");
    TimeIt(m_garbler.init_connection(), "Init connection");
    TimeIt(m_garbler.send_egtt(), "Send encrypted garbled truth tables");
    TimeIt(m_garbler.send_self_lbls(), "Send self labels");
    TimeIt(m_garbler.send_peer_lbls(), "Send peer labels");
    TimeIt(m_garbler.send_output_map(), "Send output map");
    TimeIt(m_garbler.recv_output(), "Receive output");
    parse_clean();
    timer.report();
}

void StartEvaluator(const char* garbler_ip) {
    extern FILE* yyin;
    const char* src = m_circ.c_str();
    yyin = std::tmpfile();
    std::fputs(src, yyin);
    std::rewind(yyin);
    set_evaluator(&m_evaluator);
    m_evaluator.m_peer_ip = string(garbler_ip);
    TimeIt(yyparse(), "Parsing");
    m_evaluator.check_ids();
    TimeIt(m_evaluator.build_garble_circuit(), "Build garbled circuit");
    TimeIt(m_evaluator.init_connection(), "Init connection");
    TimeIt(m_evaluator.recv_egtt(), "Send encrypted garbled truth tables");
    TimeIt(m_evaluator.recv_peer_lbls(), "Send self labels");
    TimeIt(m_evaluator.recv_self_lbls(), "Send peer labels");
    TimeIt(m_evaluator.recv_output_map(), "Send output map");
    TimeIt(m_evaluator.m_gc.evaluate(), "Evaluating");
    TimeIt(m_evaluator.recover_output(), "Recovering output");
    TimeIt(m_evaluator.send_output(), "Send output");
    parse_clean();
    timer.report();
}

const char* GetGarblerRawOutput() {
    string output;
    char* outstr = new char[2048];
    m_garbler.get_output(output);
    memcpy(outstr, output.c_str(), output.size() + 1);
    return outstr;
}
const char* GetEvaluatorRawOutput() {
    string output;
    char* outstr = new char[2048];
    m_evaluator.get_output(output);
    memcpy(outstr, output.c_str(), output.size() + 1);
    return outstr;
}

void ResetGarbler() {
    m_garbler.clear();
}

void ResetEvaluator() {
    m_evaluator.clear();
}

void CrossCheck() {
    extern FILE* yyin;
    const char* src = m_circ.c_str();
    yyin = std::tmpfile();
    std::fputs(src, yyin);
    std::rewind(yyin);
    set_evaluator(&m_evaluator);
    TimeIt(yyparse(), "Parsing");
    m_evaluator.m_gc.execute();
    m_evaluator.m_gc.garble();

    m_evaluator.m_gc.evaluate_and_check();
    for (int i = 0; i < m_evaluator.m_gc.m_out_id_vec.size(); ++i) {
        WI* wi = m_evaluator.m_gc.get_wi(m_evaluator.m_gc.m_out_id_vec[i]);
        printf("%d", wi->m_inv ? wi->m_wire->m_val ^ 1 : wi->m_wire->m_val);
    }
    printf("\n");
    
    for (int i = 0; i < m_evaluator.m_gc.m_out_id_vec.size(); ++i) {
        WI* wi = m_evaluator.m_gc.get_wi(m_evaluator.m_gc.m_out_id_vec[i]);
        GWI* gwi = m_evaluator.m_gc.get_gwi(m_evaluator.m_gc.m_out_id_vec[i]);
        if (wi->m_wire->m_val != gwi->m_wire->m_val) {
            perror("Gotcha!");
            abort();
        } else {
            printf("%d", wi->m_inv ? wi->m_wire->m_val ^ 1 : wi->m_wire->m_val);
        }
    }
    
    parse_clean();
    m_evaluator.clear();
}

static void BM_DIV(benchmark::State& state) {
    string g_circuit = "func div (int64 a, int64 b) { "
    "    int64 ret = a / b;        "
    "    return ret; }             "
    "   #definput a 120            ";
    string e_circuit =  "func div (int64 a, int64 b) { "
    "    int64 ret = a / b;        "
    "    return ret; }             "
    "   #definput b 12             ";
    
    if (fork() != 0) {
        extern FILE* yyin;
        const char* src = g_circuit.c_str();
        yyin = std::tmpfile();
        std::fputs(src, yyin);
        std::rewind(yyin);
        set_garbler(&m_garbler);
        m_garbler.m_peer_ip = string("127.0.0.1");
        TimeIt(yyparse(), "Parsing");
        m_garbler.check_ids();
        TimeIt(m_garbler.m_gc.garble(), "Garbling circuit");
        TimeIt(m_garbler.init_connection(), "Init connection");
        TimeIt(m_garbler.send_egtt(), "Send encrypted garbled truth tables");
        TimeIt(m_garbler.send_self_lbls(), "Send self labels");
        TimeIt(m_garbler.send_peer_lbls(), "Send peer labels");
        TimeIt(m_garbler.send_output_map(), "Send output map");
        TimeIt(m_garbler.recv_output(), "Receive output");
        parse_clean();
        timer.report();
    } else {
        sleep(2);
        extern FILE* yyin;
        const char* src = e_circuit.c_str();
        yyin = std::tmpfile();
        std::fputs(src, yyin);
        std::rewind(yyin);
        set_evaluator(&m_evaluator);
        m_evaluator.m_peer_ip = "127.0.0.1";
        TimeIt(yyparse(), "Parsing");
        m_evaluator.check_ids();
        TimeIt(m_evaluator.build_garble_circuit(), "Build garbled circuit");
        TimeIt(m_evaluator.init_connection(), "Init connection");
        TimeIt(m_evaluator.recv_egtt(), "Send encrypted garbled truth tables");
        TimeIt(m_evaluator.recv_peer_lbls(), "Send self labels");
        TimeIt(m_evaluator.recv_self_lbls(), "Send peer labels");
        TimeIt(m_evaluator.recv_output_map(), "Send output map");
        TimeIt(m_evaluator.m_gc.evaluate(), "Evaluating");
        TimeIt(m_evaluator.recover_output(), "Recovering output");
        TimeIt(m_evaluator.send_output(), "Send output");
        parse_clean();
        timer.report();
    }
}

void StartBenchmark() {
    int one = 1;
    char* argv = "program";
    ::benchmark::Initialize(&one, &argv);
    if (::benchmark::ReportUnrecognizedArguments(one, &argv)) return;
    ::benchmark::RunSpecifiedBenchmarks();
}

BENCHMARK(BM_DIV);

//void StartClient() {
//    seclenet_main();
//}
//
//void StartP0() {
//    seclenet_p0_main();
//}
//
//void StartP1() {
//    seclenet_p1_main();
//}
