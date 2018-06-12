/*
 * exec_relu.cc
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

TEST_F(EXECTest, Relu64)
{

    gashgc::Timer timer;
    exec_test(g_ip, e_ip, g_circ, g_dat,
              e_circ, e_dat, port, ot_port,
              "func ss_relugrad (int64 in0, int64 in1, int64 r) {       "
              "     int64 sum = in0 + in1;                              "
              "     int64 ret = 0;                                      "
              "     if (sum > 0) { ret = sum - r; }                     "
              "     else { ret = 0 - r; }                               "
              "     return ret; }                                       ",
              "#definput in0 -10"
              "#definput r -123214293843715731                          ",
              "#definput in1 -1                       ");

    mpz_class mpz1 = 1;
    mpz_class m_config_l = 1;
    mpz_class m_config_l_1 = 1;
    mpz_mul_2exp(m_config_l.get_mpz_t(), mpz1.get_mpz_t(), 64);
    mpz_mul_2exp(m_config_l_1.get_mpz_t(), mpz1.get_mpz_t(), 63);
    gmp_printf("m_config_l is %Zd\n", m_config_l);
    gmp_printf("m_config_l_1 is %Zd\n", m_config_l_1);


    mpz_class output;
    mpz_set_str(output.get_mpz_t(), output_str.c_str(), 2);
    printf("output_str = %s\n", output_str.c_str());
    gmp_printf("output is %Zd\n", output);

    mpz_class r("-123214293843715731", 10);
    mpz_class sum;
    sum = r + output;
    sum %= m_config_l;
    if (mpz_cmp(sum.get_mpz_t(), m_config_l_1.get_mpz_t()) > 0)
    {
        printf("sum is larger than half ring\n");
        sum = m_config_l - sum;
    }

    gmp_printf("sum is %Zd\n", sum);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
