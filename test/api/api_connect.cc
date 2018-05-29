/*
 * api_connect.cc -- Unit tests for api
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

#define g_ip "127.0.0.1"
#define e_ip "127.0.0.1"
#define gc_port 44778
#define ot_port 49887
#define clt_gbl_port 56443
#define clt_evl_port 46678

TEST_F(APITest, Billionaire) {

    if (fork() == 0) {
        // First child plays garbler
        GarblerParty g(gc_port, ot_port, clt_gbl_port);
        while (1) {
            g.serve();
        }

    } else if (fork() == 0){
        // Second child plays evaluator
        EvaluatorParty e(gc_port, ot_port, clt_evl_port);
        while (1) {
            e.serve();
        }

    } else {
        // Parent plays client

        // Sleep - wait for garbler and evaluator to set up
        sleep(1);

        ClientParty c(g_ip, e_ip, clt_gbl_port, clt_evl_port);

        REQUIRE_GOOD_STATUS(c.connect_gbl());
        REQUIRE_GOOD_STATUS(c.connect_evl());

        REQUIRE_GOOD_STATUS(c.load_function("billionaire", 64));

        u64 ret;
        c.symbolic_call("billionaire", 64, 10, 20, ret);
        EXPECT_EQ(0, ret);

    }

}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
