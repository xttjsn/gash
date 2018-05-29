/*
 * exec_tanh.cc -- Unit testing for tanh
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
#define g_circ         "tanh_g.circ"
#define g_dat          "tanh_g.dat"
#define e_circ         "tanh_e.circ"
#define e_dat          "tanh_e.dat"
#define port           7798
#define ot_port        43667


TEST_F(EXECTest, Tanh)
{
    /* x1:  573417318112 */
    /* x2:  1152920934562788224 */
    /* t11: 1152838907104052864 */
    /* t12: 1152838739325765632 */
    /* t21: 1152895781246667776 */
    /* t22: 1152895598626051072 */
    /* t31: 83744337430321 */
    /* t32: 81625192963822 */
    /* t41: 26212531024912 */
    /* t42: 25419687610228 */

  exec_test(g_ip,     e_ip,   g_circ, g_dat,
            e_circ,   e_dat,  port,   ot_port,
            "func tanh(int64 x1,  int64 x2,  int64 t11, int64 t12,    "
            "          int64 t21, int64 t22, int64 t31, int64 t32,    "
            "          int64 t41, int64 t42, int64 r) {               "
            " int64 ringm1 = 1152921504606846975;                     "
            " int64 x = (x1 + x2) & ringm1;                           "
            " int64 t1 = (t11 + t12) & ringm1;                        "
            " int64 t2 = (t21 + t22) & ringm1;                        "
            " int64 t3 = (t31 + t32) & ringm1;                        "
            " int64 t4 = (t41 + t42) & ringm1;                        "
            " int64 bigNA = 1152921502974759404;                      "  // bigPA mod 2^60
            " int64 bigNB = 1152921501847330489;                      "  // bigPB mod 2^60
            " int64 bigPA = 1632087572;                               "  // 1.52 * 2^30
            " int64 bigPB = 2759516487;                               "  // 2.57 * 2^30
            " int64 bigN1 = 1152921503533105152;                      "  // bigP1 mod 2^60
            " int64 bigP1 = 1073741824;                               "  // 1 * 2^30
            " int64 halfring = 576460752303423488;                    "
            " int64 ret = 0;                                          "
            " if (x > halfring)                                       "
            " {                                                       "
            "     if (x < bigNA)                                      "
            "     {                                                   "
            "         ret = (t3 + r) & ringm1;                        "
            "     } else                                              "
            "     {                                                   "
            "         if (x > bigNB)                                  "
            "         {                                               "
            "             ret = (bigN1 + r) & ringm1;                 "
            "         } else {                                        "
            "             ret = (t4 + r ) & ringm1;                   "
            "         }                                               "
            "     }                                                   "
            " } else                                                  "
            " {                                                       "
            "     if (x <= bigPA)                                     "
            "     {                                                   "
            "         ret = (t1 + r) & ringm1;                        "
            "     } else                                              "
            "     {                                                   "
            "         if (x > bigPB)                                  "
            "         {                                               "
            "             ret = (bigP1 + r) & ringm1;                 "
            "         } else {                                        "
            "             ret = (t2 + r) & ringm1;                    "
            "         }                                               "
            "     }                                                   "
            " }                                                       "
            " return ret;                                             "
            " }                                                       ",
            "#definput     x1   573417318112                          "
            "#definput     t11  1152838907104052864                   "
            "#definput     t21  1152895781246667776                   "
            "#definput     t31  83744337430321                        "
            "#definput     t41  26212531024912                        ",
            "#definput     x2   1152920934562788224                   "
            "#definput     t12  1152838739325765632                   "
            "#definput     t22  1152895598626051072                   "
            "#definput     t32  81625192963822                        "
            "#definput     t42  25419687610228                        "
            "#definput     r    427722916458                          ");
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
