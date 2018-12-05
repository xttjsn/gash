/*
 * main.cc -- Main function of gash lang
 *
 * Author: Xiaoting Tang <tang_xiaoting@brown.edu>
 * Copyright: Xiaoting Tang (2018)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
  * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <fstream>
#include "../gc/util.hh"
#include "../include/common.hh"
#include "../lang/gash_lang.hh"
#include "../gc/tcp.hh"
#include "../gc/garbled_circuit.hh"
#include "../gc/util.hh"
#include "../gc/aes.hh"
#include "../gc/ot.hh"
#include "../gc/garbler.hh"
#include "../gc/evaluator.hh"

#define EXPECT_EQ(a, b)  \
    if ((a) != (b))      \
        perror("EXPECT_EQ Failed")

#define EXPECT_EQ_with_Timer(val, expr, event_name)                        \
    timer.tic(event_name);                                                 \
    EXPECT_EQ(val, expr);                                                  \
    timer.toc()

using po::options_description;
using po::value;
using po::variables_map;
using po::store;
using po::notify;
using po::parse_command_line;

using std::ofstream;

using gashlang::run;

extern FILE* yyin;

int main(int argc, char* argv[])
{
    options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message for gash_lang")
        ("role,r", value<string>(), "role: [\"GARBLER\" | \"EVALUATOR\"]")
        ("input,i", value<string>(), "file path of input function description file")
        ("circ,c", value<string>(), "file path of output circ file")
        ("data,d", value<string>(), "file path of output data file")
        ("peer_ip,p", value<string>(), "Peer IP address (If you are garbler, the you should put evaluator's ip, and vice versa)")
        ("port,t", value<string>(), "Main port used for GC communication, must be the same and available in both garbler and evaluator's machine")
        ("otport,o", value<string>(), "Port for Oblivious Transfer, must be different than main port, and be the same and available in both garbler and evaluator's machine");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help")) {
        cout << "Usage: options description [options]\n";
        ;
        cout << desc;
        return 0;
    }

    const char* input_fname;
    const char* circ_fname;
    const char* data_fname;
    const char* role;
    const char* peer_ip;
    uint16_t port;
    uint16_t otport;

    if (!vm.count("input")) {
        cout << "Require input file" << endl;
        return 0;
    } else {
        input_fname = vm["input"].as<string>().c_str();
    }

    if (!vm.count("circ")) {
        cout << "Require circ file path" << endl;
        return 0;
    } else {
        circ_fname = vm["circ"].as<string>().c_str();
    }

    if (!vm.count("data")) {
        cout << "Require data file path" << endl;
        return 0;
    } else {
        data_fname = vm["data"].as<string>().c_str();
    }

    if (!vm.count("role")) {
        cout << "Require role" << endl;
        return 0;
    } else {
        role = vm["role"].as<string>().c_str();
    }

    if (!vm.count("peer_ip")) {
        cout << "Require peer_ip" << endl;
        return 0;
    } else {
        peer_ip = vm["peer_ip"].as<string>().c_str();
    }

    if (!vm.count("port")) {
        cout << "Require port" << endl;
        return 0;
    } else {
        port = static_cast<uint16_t>(strtoul(vm["port"].as<string>().c_str(), NULL, 10));
    }

    if (!vm.count("otport")) {
        cout << "Require otport" << endl;
        return 0;
    } else {
        otport = static_cast<uint16_t>(strtoul(vm["otport"].as<string>().c_str(), NULL, 10));
    }

    gashgc::Timer timer;
    srandom(time(0));
    if (strcmp(role, "EVALUATOR") == 0) {

        FILE* fp = fopen(input_fname, "r");
        string output_str;
        std::ofstream m_circ_stream = ofstream(circ_fname, std::ios::out | std::ios::trunc);
        std::ofstream m_data_stream = ofstream(data_fname, std::ios::out | std::ios::trunc);
        yyin = fp;
        gashlang::set_ofstream(m_circ_stream, m_data_stream);
        EXPECT_EQ_with_Timer(0, yyparse(), "Parsing");
        gashgc::Evaluator evaluator(peer_ip, port, otport, circ_fname, data_fname);
        EXPECT_EQ_with_Timer(0, evaluator.build_circ(), "Build circuit");
        EXPECT_EQ_with_Timer(0, evaluator.read_input(), "Read input");
        EXPECT_EQ_with_Timer(0, evaluator.build_garbled_circuit(), "Build garbled circuit");
        EXPECT_EQ_with_Timer(0, evaluator.init_connection(), "Init connection");
        EXPECT_EQ_with_Timer(0, evaluator.recv_egtt(), "Receive Encrypted Garbled Truth Tables");
        sleep(0.5);
        EXPECT_EQ_with_Timer(0, evaluator.recv_self_lbls(), "Receive self labels");
        EXPECT_EQ_with_Timer(0, evaluator.recv_peer_lbls(), "Receive peer labels");
        EXPECT_EQ_with_Timer(0, evaluator.evaluate_circ(), "Evaluate circuit");
        EXPECT_EQ_with_Timer(0, evaluator.recv_output_map(), "Receive output map");
        EXPECT_EQ_with_Timer(0, evaluator.recover_output(), "Recover output");
        EXPECT_EQ_with_Timer(0, evaluator.report_output(), "Report output");
        EXPECT_EQ_with_Timer(0, evaluator.send_output(), "Send output");
        evaluator.get_output(output_str);

        fclose(fp);
    }
    else if (strcmp(role, "EVALUATOR") == 0) {
        FILE* fp = fopen(input_fname, "r");
        string output_str;
        std::ofstream m_circ_stream = ofstream(circ_fname, std::ios::out | std::ios::trunc);
        std::ofstream m_data_stream = ofstream(data_fname, std::ios::out | std::ios::trunc);
        yyin = fp;
        gashlang::set_ofstream(m_circ_stream, m_data_stream);
        EXPECT_EQ_with_Timer(0, yyparse(), "Parsing");
        gashgc::Garbler garbler(peer_ip, port, otport, circ_fname, data_fname);
        EXPECT_EQ_with_Timer(0, garbler.build_circ(), "Build circuit");
        EXPECT_EQ_with_Timer(0, garbler.read_input(), "Read input");
        EXPECT_EQ_with_Timer(0, garbler.garble_circ(), "Garble circuit");
        EXPECT_EQ_with_Timer(0, garbler.init_connection(), "Init connection");
        EXPECT_EQ_with_Timer(0, garbler.send_egtt(), "Send encrypted garbled truth tables");
        EXPECT_EQ_with_Timer(0, garbler.send_peer_lbls(), "Send peer labels");
        EXPECT_EQ_with_Timer(0, garbler.send_self_lbls(), "Send self labels");
        EXPECT_EQ_with_Timer(0, garbler.send_output_map(), "Send output map");
        EXPECT_EQ_with_Timer(0, garbler.recv_output(), "Receive output");
        EXPECT_EQ_with_Timer(0, garbler.report_output(), "Report output");
        fclose(fp);
    } else {
        cout << "Invalid role, must be either \"GARBLER\" or \"EVALUATOR\"";
        return 0;
    }



}
