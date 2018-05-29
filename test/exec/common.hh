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

#define exec_test(g_ip,		e_ip,	g_circ,	g_dat, 							\
				  e_circ,	e_dat,	port, 	ot_port, 						\
				  func_src, input_g, 	input_e)            				\
    srandom(time(0));														\
    if (fork() == 0) {														\
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
        int parse_result = yyparse();										\
        EXPECT_EQ(0, parse_result);											\
        Evaluator evaluator(g_ip, port, ot_port, e_circ, e_dat);			\
        EXPECT_EQ(0, evaluator.build_circ());								\
        EXPECT_EQ(0, evaluator.read_input());								\
        EXPECT_EQ(0, evaluator.build_garbled_circuit());					\
        EXPECT_EQ(0, evaluator.init_connection());							\
        EXPECT_EQ(0, evaluator.recv_egtt());								\
        sleep(0.5);          												\
        EXPECT_EQ(0, evaluator.recv_self_lbls());							\
        EXPECT_EQ(0, evaluator.recv_peer_lbls());							\
        EXPECT_EQ(0, evaluator.evaluate_circ());							\
        EXPECT_EQ(0, evaluator.recv_output_map());							\
        EXPECT_EQ(0, evaluator.recover_output());							\
        EXPECT_EQ(0, evaluator.report_output());							\
        EXPECT_EQ(0, evaluator.send_output());								\
    } else {																\
        m_circ_stream = ofstream(g_circ, std::ios::out | std::ios::trunc);	\
        m_data_stream = ofstream(g_dat, std::ios::out | std::ios::trunc);	\
        extern FILE* yyin;													\
        const char* src = func_src											\
                          input_g;											\
        yyin = std::tmpfile();												\
        std::fputs(src, yyin);												\
        std::rewind(yyin);													\
        gashlang::set_ofstream(m_circ_stream, m_data_stream);				\
        int parse_result = yyparse();										\
        EXPECT_EQ(0, parse_result);											\
        Garbler garbler(e_ip, port, ot_port, g_circ, g_dat);				\
        EXPECT_EQ(0, garbler.build_circ());									\
        EXPECT_EQ(0, garbler.read_input());									\
        EXPECT_EQ(0, garbler.garble_circ());								\
        EXPECT_EQ(0, garbler.init_connection());							\
        EXPECT_EQ(0, garbler.send_egtt());									\
        EXPECT_EQ(0, garbler.send_peer_lbls());								\
        EXPECT_EQ(0, garbler.send_self_lbls());								\
        EXPECT_EQ(0, garbler.send_output_map());							\
        EXPECT_EQ(0, garbler.recv_output());								\
        EXPECT_EQ(0, garbler.report_output());								\
    }
