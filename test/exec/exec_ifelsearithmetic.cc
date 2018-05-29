/*
 * exec_ifelsearithmetic.cc -- Unit testing for arithmetics that have if else
 * statements in them
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
#include "common.hh"

#define g_ip           "127.0.0.1"
#define e_ip           "127.0.0.1"
#define g_circ         "ifelsearithmetic_g.circ"
#define g_dat          "ifelsearithmetic_g.dat"
#define e_circ         "ifelsearithmetic_e.circ"
#define e_dat          "ifelsearithmetic_e.dat"
#define port           7798
#define ot_port        43667

// TEST_F(EXECTest, IfElseAddSub)
// {
//     exec_test(g_ip,     e_ip,   g_circ, g_dat,
//             e_circ,   e_dat,  port,   ot_port,
//             "func ifelse(int64 a, int64 b) { "
//             " int64 ret = 0;                      "
//             " if (a > b) { ret = a + b; }         "
//             " else { ret = a - b; }               "
//             " return ret; }                       ",
//             "#definput     b    15                ",
//             "#definput     a    14                ");
// }

// TEST_F(EXECTest, IfElseAssgn)
// {
//   exec_test(g_ip,     e_ip,   g_circ, g_dat,
//             e_circ,   e_dat,  port,   ot_port,
//             "func ifelse(int4 a, int4 b) {      "
//             " int4 ret = 0;                             "
//             " int4 halfring = 5                  ;      "   // halfring = 12^59
//             " int4 c = a + b;                           "
//             " if ( halfring < c) { ret = 2; }           "
//             " else { ret = 5; }                         "
//             " return ret; }                              ",
//             "#definput     b    3                       ",
//             "#definput     a    2                       ");
// }

TEST_F(EXECTest, NestedIf)
{
  exec_test(g_ip,     e_ip,   g_circ, g_dat,
            e_circ,   e_dat,  port,   ot_port,
            "func ifelse(int4 a, int4 b) {      "
            " int4 ret = 0;                             "
            " int4 c = a + b;                           "
            " int4 d = 6;                               "
            " if ( a > d ) { ret = 2; }                 "
            " else { if (a > b) { ret = 3; }            "
            "        else  { ret = 1; }                 "
            " }                                         "
            " return ret; }                             ",
            "#definput     b    3                       ",
            "#definput     a    4                       ");
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
