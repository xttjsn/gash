/*
 * common.hh -- Common directives for testing executions
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

#include "../../gc/util.hh"

#define EXPECT_EQ_with_Timer(val, expr, event_name)                        \
    timer.tic(event_name);                                                 \
    EXPECT_EQ(val, expr);                                                  \
    timer.toc()

#define exec_test(g_ip,		e_ip,	g_circ,	g_dat, 							\
				  e_circ,	e_dat,	port, 	ot_port, 						\
				  func_src, input_g, 	input_e)            				\
    srandom(time(0));														\
    string output_str;                          \
    int role;                                    \
    if (fork() == 0) {														\
        role = 1;                                 \
        sleep(0.5);															\
        m_circ_stream = ofstream(e_circ, std::ios::out | std::ios::trunc);	\
        m_data_stream = ofstream(e_dat, std::ios::out | std::ios::trunc);	\
        extern FILE* yyin;													\
        const char* src = func_src											\
                          input_e;											\
        yyin = std::tmpfile();												\
        std::fputs(src, yyin);												\
        std::rewind(yyin);													\
        gashlang::set_ofstream(m_circ_stream, m_data_stream);				\
        EXPECT_EQ_with_Timer(0, yyparse(), "Parsing");                          \
        Evaluator evaluator(g_ip, port, ot_port, e_circ, e_dat);			\
        EXPECT_EQ_with_Timer(0, evaluator.build_circ(), "Build circuit");								\
        EXPECT_EQ_with_Timer(0, evaluator.read_input(), "Read input");								\
        EXPECT_EQ_with_Timer(0, evaluator.build_garbled_circuit(), "Build garbled circuit");    \
        EXPECT_EQ_with_Timer(0, evaluator.init_connection(), "Init connection");							\
        EXPECT_EQ_with_Timer(0, evaluator.recv_egtt(), "Receive Encrypted Garbled Truth Tables");								\
        sleep(0.5);          												\
        EXPECT_EQ_with_Timer(0, evaluator.recv_self_lbls(), "Receive self labels");							\
        EXPECT_EQ_with_Timer(0, evaluator.recv_peer_lbls(), "Receive peer labels");							\
        EXPECT_EQ_with_Timer(0, evaluator.evaluate_circ(), "Evaluate circuit");							\
        EXPECT_EQ_with_Timer(0, evaluator.recv_output_map(), "Receive output map");							\
        EXPECT_EQ_with_Timer(0, evaluator.recover_output(), "Recover output");							\
        EXPECT_EQ_with_Timer(0, evaluator.report_output(), "Report output");							\
        EXPECT_EQ_with_Timer(0, evaluator.send_output(), "Send output");								\
        evaluator.get_output(output_str);                                      \
    } else {																\
        role = 0;                                                       \
        m_circ_stream = ofstream(g_circ, std::ios::out | std::ios::trunc);	\
        m_data_stream = ofstream(g_dat, std::ios::out | std::ios::trunc);	\
        extern FILE* yyin;													\
        const char* src = func_src											\
                          input_g;											\
        yyin = std::tmpfile();												\
        std::fputs(src, yyin);												\
        std::rewind(yyin);													\
        gashlang::set_ofstream(m_circ_stream, m_data_stream);				\
        EXPECT_EQ_with_Timer(0, yyparse(), "Parsing");											\
        Garbler garbler(e_ip, port, ot_port, g_circ, g_dat);				\
        EXPECT_EQ_with_Timer(0, garbler.build_circ(), "Build circuit");									\
        EXPECT_EQ_with_Timer(0, garbler.read_input(), "Read input");									\
        EXPECT_EQ_with_Timer(0, garbler.garble_circ(), "Garble circuit");								\
        EXPECT_EQ_with_Timer(0, garbler.init_connection(), "Init connection");							\
        EXPECT_EQ_with_Timer(0, garbler.send_egtt(), "Send encrypted garbled truth tables");									\
        EXPECT_EQ_with_Timer(0, garbler.send_peer_lbls(), "Send peer labels");								\
        EXPECT_EQ_with_Timer(0, garbler.send_self_lbls(), "Send self labels");								\
        EXPECT_EQ_with_Timer(0, garbler.send_output_map(), "Send output map");							\
        EXPECT_EQ_with_Timer(0, garbler.recv_output(), "Receive output");								\
        EXPECT_EQ_with_Timer(0, garbler.report_output(), "Report output");								\
    }
