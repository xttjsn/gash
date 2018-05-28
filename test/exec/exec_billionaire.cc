/*
 * exec_add.cc -- Unit testing for executing the generated circuit
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

#include "../include/common.hh"

#define g_ip           "127.0.0.1"
#define e_ip           "127.0.0.1"
#define g_circ         "billionaire_g.circ"
#define g_dat          "billionaire_g.dat"
#define e_circ         "billionaire_e.circ"
#define e_dat          "billionaire_e.dat"
#define port           7798
#define ot_port        43667

TEST_F(EXECTest, Billionaire64)
{

    /// Generate random port
    srandom(time(0));

    if (fork() == 0) {
        /// Child plays evaluator
        sleep(0.5);
        m_circ_stream = ofstream(e_circ, std::ios::out | std::ios::trunc);
        m_data_stream = ofstream(e_dat, std::ios::out | std::ios::trunc);
        extern FILE* yyin;
        const char* src = "func billionaire(int64 a, int64 b) { "
                          " int1 ret = 0;                       "
                          " if (a > b) { ret = 1;             } "
                          " return ret;                       } "
                          "#definput     b    13                ";

        yyin = std::tmpfile();
        std::fputs(src, yyin);
        std::rewind(yyin);
        gashlang::set_ofstream(m_circ_stream, m_data_stream);

        int parse_result = yyparse();

        EXPECT_EQ(0, parse_result);

        Evaluator evaluator(g_ip, port, ot_port, e_circ, e_dat);

        EXPECT_EQ(0, evaluator.build_circ());
        EXPECT_EQ(0, evaluator.read_input());
        EXPECT_EQ(0, evaluator.build_garbled_circuit());
        EXPECT_EQ(0, evaluator.init_connection());
        EXPECT_EQ(0, evaluator.recv_egtt());
        sleep(0.5);          // Give server some time to setup
        EXPECT_EQ(0, evaluator.recv_self_lbls());
        EXPECT_EQ(0, evaluator.recv_peer_lbls());
        EXPECT_EQ(0, evaluator.evaluate_circ());
        EXPECT_EQ(0, evaluator.recv_output_map());
        EXPECT_EQ(0, evaluator.recover_output());
        EXPECT_EQ(0, evaluator.report_output());
        EXPECT_EQ(0, evaluator.send_output());
    } else {
        /// Parent plays garbler

        /// Parsing
        m_circ_stream = ofstream(g_circ, std::ios::out | std::ios::trunc);
        m_data_stream = ofstream(g_dat, std::ios::out | std::ios::trunc);
        extern FILE* yyin;
        const char* src = "func billionaire(int64 a, int64 b) { "
                          " int1 ret = 0;                       "
                          " if (a > b) { ret = 1;             } "
                          " return ret;                       } "
                          "#definput     a    14                ";

        yyin = std::tmpfile();
        std::fputs(src, yyin);
        std::rewind(yyin);
        gashlang::set_ofstream(m_circ_stream, m_data_stream);

        int parse_result = yyparse();

        EXPECT_EQ(0, parse_result);

        Garbler garbler(e_ip, port, ot_port, g_circ, g_dat);

        EXPECT_EQ(0, garbler.build_circ());
        EXPECT_EQ(0, garbler.read_input());
        EXPECT_EQ(0, garbler.garble_circ());
        EXPECT_EQ(0, garbler.init_connection());
        EXPECT_EQ(0, garbler.send_egtt());
        EXPECT_EQ(0, garbler.send_peer_lbls());
        EXPECT_EQ(0, garbler.send_self_lbls());
        EXPECT_EQ(0, garbler.send_output_map());
        EXPECT_EQ(0, garbler.recv_output());
        EXPECT_EQ(0, garbler.report_output());
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
