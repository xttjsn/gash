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

    if (fork() != 0) {
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
    } else {

        if (fork() != 0) {
            // First child is peer0 / garbler
            sleep(1);

            mpz_class a0;
            mpz_class b0;
            gash_config_init();
            gash_init_as_garbler(e_ip);
            gash_connect_peer();
            gash_ss_garbler_init(c_ip);
            gash_ss_recv_share(a0);
            gash_ss_recv_share(b0);
            gash_ss_share_triplet_slave();

            mpz_class y0 = gash_ss_mul(a0, b0);
            gash_ss_recon_slave(y0);

        } else {
            // Second child is peer1 / evaluator
            sleep(3);

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
    }
}

int main(int argc, char *argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
