/*
 * exec_neg.cc
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
#define g_circ         "neg_g.circ"
#define g_dat          "neg_g.dat"
#define e_circ         "neg_e.circ"
#define e_dat          "neg_e.dat"
#define port           7798
#define ot_port        43667

TEST_F(EXECTest, Sub64)
{

    gashgc::Timer timer;
    exec_test(g_ip,     e_ip,   g_circ, g_dat,
            e_circ,   e_dat,  port,   ot_port,
            "func neg(int64 a, int64 b) {       "
            "    return a + b;                  "
            "}                                  ",
              "#definput     a    -1720676905983058014              ",
              "#definput     b    1720676905983068014                ");

}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
