/*
 * api_mul.cc -- Unit tests for api
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

#define g_ip "127.0.0.1"
#define e_ip "127.0.0.1"
#define c_ip "127.0.0.1"
#define gc_port 44778
#define ot_port 49887
#define clt_gbl_port 56443
#define clt_evl_port 46678

typedef vector<int> IntVec;

gmp_randclass gmp_prn(gmp_randinit_default);

TEST_F(APITest, SecMul) {

    // Second child is peer1 / evaluator

    mpz_class a1;
    mpz_class b1;
    gash_config_init();
    gash_init_as_evaluator(g_ip);
    gash_connect_peer();
    gash_ss_evaluator_init(c_ip, g_ip);
    gash_ss_recv_share(a1);
    gash_ss_recv_share(b1);
    gash_ss_share_triplet_slave();

    mpz_class y1 = gash_ss_mul(a1, b1);
    gash_ss_recon_slave(y1);
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
