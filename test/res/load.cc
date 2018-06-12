/*
 * load.cc -- Test circuit loading
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
#include "../../res/funcs.hh"

TEST(FuncLoadTest, HasFunc) {
    EXPECT_EQ(1, gashres::FuncRegistry::instance().has_func("add"));
    EXPECT_EQ(1, gashres::FuncRegistry::instance().has_func("billionaire"));
    EXPECT_EQ(1, gashres::FuncRegistry::instance().has_func("sub"));
    EXPECT_EQ(1, gashres::FuncRegistry::instance().has_func("relu"));
    EXPECT_EQ(1, gashres::FuncRegistry::instance().has_func("ss_relu"));
    EXPECT_EQ(1, gashres::FuncRegistry::instance().has_func("ss_relugrad"));
}

TEST(FuncLoadTest, LoadCirc) {
    string circ;
    EXPECT_EQ(0, LOAD_CIRCUIT("add", 64, circ));
    printf("-------add circuit--------\n %s\n -------- end add circuit\
    ----------", circ.c_str());

    EXPECT_EQ(0, LOAD_CIRCUIT("sub", 64, circ));
    printf("-------sub circuit--------\n %s\n -------- end sub circuit\
    ----------", circ.c_str());

    EXPECT_EQ(0, LOAD_CIRCUIT("ss_relu", 64, circ));
    printf("-------ss_relu circuit--------\n %s\n -------- end ss_relu circuit\
    ----------", circ.c_str());

    EXPECT_EQ(0, LOAD_CIRCUIT("ss_relugrad", 64, circ));
    printf("-------ss_relugrad circuit--------\n %s\n -------- end ss_relugrad circuit\
    ----------", circ.c_str());
}

TEST(FuncLoadTest, LoadData) {
    string data;
    EXPECT_EQ(0, LOAD_DATA("in0", "12345", data));
    printf("data: %s\n", data.c_str());
}

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
