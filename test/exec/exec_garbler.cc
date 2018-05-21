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

#define g_port        41554
#define g_ot_port     46778
#define g_circ        "add4_g.circ"
#define g_dat         "add4_g.dat"

TEST_F(EXECTest, BuildCirc) {

  /// Parsing
  m_circ_stream = ofstream(g_circ, std::ios::out | std::ios::trunc);
  m_data_stream = ofstream(g_dat, std::ios::out | std::ios::trunc);
  extern FILE* yyin;
  const char* src = "func add(int4 a, int4 b) {      "
                    "    return a + b;               "
                    "}                               "
                    "#definput     a    0            ";

  yyin = std::tmpfile();
  std::fputs(src, yyin);
  std::rewind(yyin);
  gashlang::set_ofstream(m_circ_stream, m_data_stream);

  int parse_result = yyparse();

  EXPECT_EQ(0, parse_result);

  Garbler garbler(g_port, g_ot_port, string(g_circ), string(g_dat));

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

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
