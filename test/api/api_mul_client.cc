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

    // Parent is the client
    mpz_class a = -10000;
    mpz_class b = 33;
    mpz_class y;
    gmp_printf("a = %Zd\n", a);
    gmp_printf("b = %Zd\n", b);
    gash_config_init();
    gash_ss_client_init();
    gash_ss_send_share(a);
    gash_ss_send_share(b);
    gash_ss_generate_triplet();
    gash_ss_share_triplet_master();
    gash_ss_recon_master(y);

    printf("a * b = %s\n", y.get_str(10).c_str());
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
