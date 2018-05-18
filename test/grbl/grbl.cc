/*
 * grbl.cc -- Unit test for garbling functions
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

#define NTEST 20

namespace gashgc {
    extern block AESkey;
}

using gashgc::aes_decrypt128;
using gashgc::aes_encrypt128;
using gashgc::AESkey;
using gashgc::block2hex;
using gashgc::block_eq;
using gashgc::decrypt;
using gashgc::EGTT;
using gashgc::encrypt;
using gashgc::eval_bgate;
using gashgc::get_lsb;
using gashgc::new_tweak;
using gashgc::random_block;
using gashgc::set_lsb;
using gashgc::u32;
using gashgc::xor_block;

TEST_F(GRBLTest, REndsWith1)
{

    for (int i = 0; i < NTEST; ++i) {
        m_gc.init();
        EXPECT_EQ(1, get_lsb(m_gc.m_R));
    }
}

TEST_F(GRBLTest, TwoLablesHaveOppositeLSB)
{
    WI* wi = new WI(100, 0);
    GWI* gwi = new GWI(wi);

    for (int i = 0; i < NTEST; ++i) {
        gwi->garble(m_gc.m_R);

        // cout << "R:"    << block2hex(m_gc.m_R) << endl;
        // cout << "lbl0:" << block2hex(gwi->get_lbl0()) << endl;
        // cout << "lbl1:" << block2hex(gwi->get_lbl1()) << endl;

        EXPECT_EQ(1, get_lsb(gwi->get_lbl0()) ^ get_lsb(gwi->get_lbl1()));
    }

    delete wi;
    delete gwi;
}

TEST_F(GRBLTest, CorrectDecryption)
{

    block a = random_block();
    block b = random_block();
    block c = random_block();
    block cipher;
    block dec;
    block tweak;

    for (int i = 0; i < NTEST; ++i) {
        tweak = new_tweak(i);
        cipher = encrypt(a, b, tweak, c, AESkey);
        dec = decrypt(a, b, tweak, cipher, AESkey);
        EXPECT_EQ(1, block_eq(c, dec));
    }
}

TEST_F(GRBLTest, CorrectEGTTDecryption)
{

    GG* gg;
    u32 id = 19;
    block ZERO = getZEROblock();
    block correct_lbl;

    for (int in0val = 0; in0val < 2; in0val++) {
        for (int in1val = 1; in1val < 2; in1val++) {
            for (int in0inv = 0; in0inv < 2; in0inv++) {
                for (int in1inv = 1; in1inv < 2; in1inv++) {
                    for (int outinv = 0; outinv < 2; outinv++) {

                        /// Garbling, or encryption
                        {
                            WI* win0 = new WI(100, in0inv);
                            GWI* gin0 = new GWI(win0);
                            WI* win1 = new WI(101, in1inv);
                            GWI* gin1 = new GWI(win1);
                            WI* wout = new WI(102, outinv);
                            GWI* gout = new GWI(wout);
                            int func = funcAND;

                            block gtt[4];
                            block lbl0;
                            block lbl1;
                            block lbl;
                            block tweak;
                            EGTT* egtt;

                            int first_row_smtc;

                            gin0->garble(m_gc.m_R);
                            gin1->garble(m_gc.m_R);

                            if (func == funcXOR) {

                                lbl0 = xor_block(gin0->get_lbl0(), gin1->get_lbl0());
                                gout->set_lbl0(lbl0);

                                lbl1 = xor_block(lbl0, m_gc.m_R);
                                gout->set_lbl1(lbl1);

                                gg = new GG(1, gin0, gin1, gout);

                            } else {

                                tweak = new_tweak(id);

                                first_row_smtc = eval_bgate(gin0->get_smtc_w_lsb(0), gin1->get_smtc_w_lsb(0), func);

                                lbl = encrypt(gin0->get_lbl_w_lsb(0), gin1->get_lbl_w_lsb(0), tweak, ZERO, AESkey);

                                gout->set_lbl_w_smtc(lbl, first_row_smtc);
                                gout->set_lbl_w_smtc(xor_block(lbl, m_gc.m_R), first_row_smtc ^ 1);

                                lbl0 = gout->get_lbl_w_smtc(0);
                                lbl1 = gout->get_lbl_w_smtc(1);

                                gtt[1] = encrypt(gin0->get_lbl_w_lsb(1),
                                    gin1->get_lbl_w_lsb(0),
                                    tweak,
                                    gout->get_lbl_w_smtc(eval_bgate(gin0->get_smtc_w_lsb(1),
                                        gin1->get_smtc_w_lsb(0),
                                        func)),
                                    AESkey);

                                gtt[2] = encrypt(gin0->get_lbl_w_lsb(0),
                                    gin1->get_lbl_w_lsb(1),
                                    tweak,
                                    gout->get_lbl_w_smtc(eval_bgate(gin0->get_smtc_w_lsb(0),
                                        gin1->get_smtc_w_lsb(1),
                                        func)),
                                    AESkey);

                                gtt[3] = encrypt(gin0->get_lbl_w_lsb(1),
                                    gin1->get_lbl_w_lsb(1),
                                    tweak,
                                    gout->get_lbl_w_smtc(eval_bgate(gin0->get_smtc_w_lsb(1),
                                        gin1->get_smtc_w_lsb(1),
                                        func)),
                                    AESkey);

                                egtt = new EGTT(gtt[1], gtt[2], gtt[3]);
                                gg = new GG(0, gin0, gin1, gout, egtt);
                            }

                            /// Compute the correct label
                            correct_lbl = eval_bgate(in0val, in1val, func) == 0 ? lbl0 : lbl1;
                        }

                        /// Evaluation, or decryption
                        {
                            block tweak;
                            block lbl;

                            GWI* gin0;
                            GWI* gin1;
                            GWI* gout;

                            int select;

                            gin0 = gg->m_in0;
                            gin1 = gg->m_in1;
                            gout = gg->m_out;

                            /// Mimic oblivious transfer
                            gin0->set_lbl(gin0->get_lbl_w_smtc(in0val));
                            gin1->set_lbl(gin1->get_lbl_w_smtc(in1val));
                            /// End mimic oblivious transfer

                            if (gg->m_is_xor) {

                                lbl = xor_block(gin0->get_lbl(), gin1->get_lbl());

                            } else {

                                select = get_lsb(gin0->get_lbl()) + (get_lsb(gin1->get_lbl()) << 1);
                                tweak = new_tweak(id);

                                if (select == 0) {

                                    lbl = encrypt(gin0->get_lbl(), gin1->get_lbl(), tweak, ZERO, AESkey);

                                } else {

                                    lbl = decrypt(gin0->get_lbl(),
                                        gin1->get_lbl(),
                                        tweak,
                                        gg->m_egtt->get_row(select),
                                        AESkey);
                                }
                            }

                            /// Check whether lbl is the correct label
                            EXPECT_EQ(1, block_eq(lbl, correct_lbl));
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
