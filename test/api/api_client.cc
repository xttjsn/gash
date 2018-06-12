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
#include "../../api/gash.hh"
#include <gmpxx.h>

#define g_ip "127.0.0.1"
#define e_ip "127.0.0.1"
#define c_ip "127.0.0.1"
#define gc_port 44778
#define ot_port 49887
#define clt_gbl_port 56443
#define clt_evl_port 46678

typedef vector<int> IntVec;

gmp_randclass gmp_prn(gmp_randinit_default);

TEST_F(APITest, Relu) {

    mpz_class y = 10000;
    gash_config_init();
    gash_ss_client_init();
    gash_ss_send_share(y);
    gash_ss_recon_master(y);

    printf("relu(x) = %s\n", y.get_str(10).c_str());
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
