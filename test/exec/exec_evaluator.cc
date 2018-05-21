/*
 * exec.cc -- Unit testing for executing the generated circuit
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

#define g_ip          "127.0.0.1"
#define g_port        41554
#define g_ot_port     46778
#define e_circ        "add4_e.circ"
#define e_dat         "add4_e.dat"

TEST_F(EXECTest, BuildCirc) {

  /// Parsing
  m_circ_stream = ofstream(e_circ, std::ios::out | std::ios::trunc);
  m_data_stream = ofstream(e_dat, std::ios::out | std::ios::trunc);
  extern FILE* yyin;
  const char* src = "func add(int4 a, int4 b) {      "
                    "    return a + b;               "
                    "}                               "
                    "#definput     b    1            ";

  yyin = std::tmpfile();
  std::fputs(src, yyin);
  std::rewind(yyin);
  gashlang::set_ofstream(m_circ_stream, m_data_stream);

  int parse_result = yyparse();

  EXPECT_EQ(0, parse_result);

  Evaluator evaluator(g_ip, g_port, g_ot_port, string(e_circ), string(e_dat));

  EXPECT_EQ(0, evaluator.build_circ());
  EXPECT_EQ(0, evaluator.read_input());
  EXPECT_EQ(0, evaluator.build_garbled_circuit());
  EXPECT_EQ(0, evaluator.init_connection());
  EXPECT_EQ(0, evaluator.recv_egtt());
  EXPECT_EQ(0, evaluator.recv_self_lbls());
  EXPECT_EQ(0, evaluator.recv_peer_lbls());
  EXPECT_EQ(0, evaluator.evaluate_circ());
  EXPECT_EQ(0, evaluator.recv_output_map());
  EXPECT_EQ(0, evaluator.recover_output());
  EXPECT_EQ(0, evaluator.report_output());
  EXPECT_EQ(0, evaluator.send_output());
}

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
