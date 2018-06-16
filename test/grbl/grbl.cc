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
#include "../../gc/util.hh"

#define NTEST 20
using gashgc::srand_sse;


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
    block theother_lbl;
    int funcs[] = {funcAND, funcXOR, funcOR};
    int func;

    for (int i = 0; i < 3; ++i) {
        for (int in0val = 0; in0val < 2; in0val++) {
            for (int in1val = 0; in1val < 2; in1val++) {
                for (int in0inv = 0; in0inv < 2; in0inv++) {
                    for (int in1inv = 0; in1inv < 2; in1inv++) {
                        for (int outinv = 0; outinv < 2; outinv++) {

                            /// Garbling, or encryption
                            {
                                WI* win0 = new WI(100, in0inv);
                                GWI* gin0 = new GWI(win0);
                                WI* win1 = new WI(101, in1inv);
                                GWI* gin1 = new GWI(win1);
                                WI* wout = new WI(102, outinv);
                                GWI* gout = new GWI(wout);
                                func = funcs[i];

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
                                theother_lbl = eval_bgate(in0val, in1val, func) == 0 ? lbl1 : lbl0;
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
                                if (1 != block_eq(lbl, correct_lbl)) {
                                    cout << "func:" << func << endl;
                                    cout << "in0val:" << in0val << endl;
                                    cout << "in1val:" << in1val << endl;
                                    cout << "in0inv:" << in0inv << endl;
                                    cout << "in1inv:" << in1inv << endl;
                                    cout << "outinv:" << outinv << endl;
                                    cout << "lbl: " << block2hex(lbl) << endl;
                                    cout << "correct_lbl: " << block2hex(correct_lbl) << endl;
                                    cout << "the other lbl: " << block2hex(theother_lbl) << endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

TEST_F(GRBLTest, CorrectADD) {

    GC gc;
    GG* gg_xor0, *gg_xor1, *gg_and0, *gg_and1, *gg_or;
    u32 id = 19;
    block ZERO = getZEROblock();
    block correct_lbl_xor0, correct_lbl_xor1, correct_lbl_and0, correct_lbl_and1, correct_lbl_or;
    block theother_lbl_xor0, theother_lbl_xor1, theother_lbl_and0, theother_lbl_and1, theother_lbl_or;
    int func;

    srand_sse(time(0));

        for (int in0val = 0; in0val < 2; in0val++) {
            for (int in1val = 0; in1val < 2; in1val++) {
                for (int in0inv = 0; in0inv < 2; in0inv++) {
                    for (int in1inv = 0; in1inv < 2; in1inv++) {
                        for (int cval = 0; cval < 2; cval++) {
                            for (int cinv = 0;  cinv < 2; cinv++) {

                                /// Garbling the first XOR, or encryption
                                {
                                    // The first XOR
                                    WI* win0 = new WI(100, in0inv);
                                    GWI* gin0 = new GWI(win0);
                                    WI* win1 = new WI(101, in1inv);
                                    GWI* gin1 = new GWI(win1);
                                    WI* wout = new WI(102, 0);
                                    GWI* gout = new GWI(wout);
                                    func = funcXOR;
                                    gc.add_gwi(gin0);
                                    gc.add_gwi(gin1);
                                    gc.add_gwi(gout);

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

                                        gg_xor0 = new GG(1, gin0, gin1, gout);

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
                                        gg_xor0 = new GG(0, gin0, gin1, gout, egtt);
                                    }

                                    /// Compute the correct label
                                    correct_lbl_xor0 = eval_bgate(in0val, in1val, func) == 0 ? lbl0 : lbl1;
                                    theother_lbl_xor0 = eval_bgate(in0val, in1val, func) == 0 ? lbl1 : lbl0;

                                    gc.add_gg(gg_xor0);
                                }

                                /// Garbling the second XOR
                                {
                                    // The second XOR
                                    GWI* gin0 = gc.get_gwi(102);
                                    WI* win1 = new WI(103, cinv);
                                    GWI* gin1 = new GWI(win1);
                                    WI* wout = new WI(104, 0);
                                    GWI* gout = new GWI(wout);
                                    func = funcXOR;
                                    gc.add_gwi(gin1);
                                    gc.add_gwi(gout);

                                    block gtt[4];
                                    block lbl0;
                                    block lbl1;
                                    block lbl;
                                    block tweak;
                                    EGTT* egtt;

                                    int first_row_smtc;

                                    gin1->garble(m_gc.m_R);

                                    if (func == funcXOR) {

                                        lbl0 = xor_block(gin0->get_lbl0(), gin1->get_lbl0());
                                        gout->set_lbl0(lbl0);

                                        lbl1 = xor_block(lbl0, m_gc.m_R);
                                        gout->set_lbl1(lbl1);

                                        gg_xor1 = new GG(1, gin0, gin1, gout);

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
                                        gg_xor1 = new GG(0, gin0, gin1, gout, egtt);
                                    }

                                    /// Compute the correct label
                                    correct_lbl_xor1 = eval_bgate(in0val ^ in1val, cval, func) == 0 ? lbl0 : lbl1;
                                    theother_lbl_xor1 = eval_bgate(in0val, in1val, func) == 0 ? lbl1 : lbl0;

                                    gc.add_gg(gg_xor1);

                                }

                                /// Garbling first AND
                                {
                                    GWI* gin0 = gc.get_gwi(100);
                                    GWI* gin1 = gc.get_gwi(101);
                                    WI* wout = new WI(105, 0);
                                    GWI* gout = new GWI(wout);
                                    func = funcAND;
                                    gc.add_gwi(gout);

                                    block gtt[4];
                                    block lbl0;
                                    block lbl1;
                                    block lbl;
                                    block tweak;
                                    EGTT* egtt;

                                    int first_row_smtc;

                                    if (func == funcXOR) {

                                        lbl0 = xor_block(gin0->get_lbl0(), gin1->get_lbl0());
                                        gout->set_lbl0(lbl0);

                                        lbl1 = xor_block(lbl0, m_gc.m_R);
                                        gout->set_lbl1(lbl1);

                                        gg_and0 = new GG(1, gin0, gin1, gout);

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
                                        gg_and0 = new GG(0, gin0, gin1, gout, egtt);
                                    }

                                    /// Compute the correct label
                                    correct_lbl_and0 = eval_bgate(in0val, in1val, func) == 0 ? lbl0 : lbl1;
                                    theother_lbl_and0 = eval_bgate(in0val, in1val, func) == 0 ? lbl1 : lbl0;

                                    gc.add_gg(gg_and0);
                                }

                                /// Garbling second AND
                                {
                                    GWI* gin0 = gc.get_gwi(102);
                                    GWI* gin1 = gc.get_gwi(103);
                                    WI* wout = new WI(106, 0);
                                    GWI* gout = new GWI(wout);
                                    func = funcAND;
                                    gc.add_gwi(gout);

                                    block gtt[4];
                                    block lbl0;
                                    block lbl1;
                                    block lbl;
                                    block tweak;
                                    EGTT* egtt;

                                    int first_row_smtc;

                                    if (func == funcXOR) {

                                        lbl0 = xor_block(gin0->get_lbl0(), gin1->get_lbl0());
                                        gout->set_lbl0(lbl0);

                                        lbl1 = xor_block(lbl0, m_gc.m_R);
                                        gout->set_lbl1(lbl1);

                                        gg_and1 = new GG(1, gin0, gin1, gout);

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
                                        gg_and1 = new GG(0, gin0, gin1, gout, egtt);
                                    }

                                    /// Compute the correct label
                                    correct_lbl_and1 = eval_bgate(in0val ^ in1val, cval, func) == 0 ? lbl0 : lbl1;
                                    theother_lbl_and1 = eval_bgate(in0val, in1val, func) == 0 ? lbl1 : lbl0;

                                    gc.add_gg(gg_and1);
                                }

                                /// Garbling OR
                                {
                                    GWI* gin0 = gc.get_gwi(105);
                                    GWI* gin1 = gc.get_gwi(106);
                                    WI* wout = new WI(107, 0);
                                    GWI* gout = new GWI(wout);
                                    func = funcOR;
                                    gc.add_gwi(gout);

                                    block gtt[4];
                                    block lbl0;
                                    block lbl1;
                                    block lbl;
                                    block tweak;
                                    EGTT* egtt;

                                    int first_row_smtc;

                                    if (func == funcXOR) {

                                        lbl0 = xor_block(gin0->get_lbl0(), gin1->get_lbl0());
                                        gout->set_lbl0(lbl0);

                                        lbl1 = xor_block(lbl0, m_gc.m_R);
                                        gout->set_lbl1(lbl1);

                                        gg_or = new GG(1, gin0, gin1, gout);

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
                                        gg_or = new GG(0, gin0, gin1, gout, egtt);
                                    }

                                    /// Compute the correct label
                                    correct_lbl_or = eval_bgate(in0val & in1val, ( in0val ^ in1val ) & cval, func) == 0 ? lbl0 : lbl1;
                                    theother_lbl_or = eval_bgate(in0val, in1val, func) == 0 ? lbl1 : lbl0;

                                    gc.add_gg(gg_or);
                                }

                                /// Evaluate the first XOR
                                {
                                    block tweak;
                                    block lbl;

                                    GWI* gin0;
                                    GWI* gin1;
                                    GWI* gout;

                                    int select;

                                    gg_xor0 = gc.get_gg(102);
                                    gin0 = gg_xor0->m_in0;
                                    gin1 = gg_xor0->m_in1;
                                    gout = gg_xor0->m_out;

                                    /// Mimic oblivious transfer
                                    gin0->set_lbl(gin0->get_lbl_w_smtc(in0val));
                                    gin1->set_lbl(gin1->get_lbl_w_smtc(in1val));
                                    /// End mimic oblivious transfer

                                    if (gg_xor0->m_is_xor) {

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
                                                          gg_xor0->m_egtt->get_row(select),
                                                          AESkey);
                                        }
                                    }
                                    gout->set_lbl(lbl);

                                    /// Check whether lbl is the correct label
                                    EXPECT_EQ(1, block_eq(lbl, correct_lbl_xor0));
                                    if (block_eq(lbl, correct_lbl_xor0) != 1) {
                                        cout << "lbl: " << block2hex(lbl) << endl;
                                        cout << "Correct lbl: " << block2hex(correct_lbl_xor0) << endl;
                                        cout << "The other label: " << block2hex(theother_lbl_xor0) << endl;
                                    }
                                }

                                /// Evaluate the second XOR
                                {
                                    block tweak;
                                    block lbl;

                                    GWI* gin0;
                                    GWI* gin1;
                                    GWI* gout;

                                    int select;

                                    gg_xor1 = gc.get_gg(104);
                                    gin0 = gg_xor1->m_in0;
                                    gin1 = gg_xor1->m_in1;
                                    gout = gg_xor1->m_out;

                                    /// Mimic oblivious transfer
                                    gin1->set_lbl(gin1->get_lbl_w_smtc(cval));
                                    /// End mimic oblivious transfer

                                    if (gg_xor1->m_is_xor) {

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
                                                          gg_xor1->m_egtt->get_row(select),
                                                          AESkey);
                                        }
                                    }
                                    gout->set_lbl(lbl);

                                    /// Check whether lbl is the correct label
                                    EXPECT_EQ(1, block_eq(lbl, correct_lbl_xor1));
                                    if (block_eq(lbl, correct_lbl_xor1) != 1) {
                                        cout << "lbl: " << block2hex(lbl) << endl;
                                        cout << "Correct lbl: " << block2hex(correct_lbl_xor1) << endl;
                                        cout << "The other label: " << block2hex(theother_lbl_xor1) << endl;
                                    }
                                }

                                /// Evaluate the first AND
                                {
                                    block tweak;
                                    block lbl;

                                    GWI* gin0;
                                    GWI* gin1;
                                    GWI* gout;

                                    int select;

                                    gg_and0 = gc.get_gg(105);
                                    gin0 = gg_and0->m_in0;
                                    gin1 = gg_and0->m_in1;
                                    gout = gg_and0->m_out;

                                    /// Mimic oblivious transfer
                                    gin0->set_lbl(gin0->get_lbl_w_smtc(in0val));
                                    gin1->set_lbl(gin1->get_lbl_w_smtc(in1val));
                                    /// End mimic oblivious transfer

                                    if (gg_and0->m_is_xor) {

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
                                                          gg_and0->m_egtt->get_row(select),
                                                          AESkey);
                                        }
                                    }
                                    gout->set_lbl(lbl);

                                    /// Check whether lbl is the correct label
                                    EXPECT_EQ(1, block_eq(lbl, correct_lbl_and0));
                                    if (block_eq(lbl, correct_lbl_and0) != 1) {
                                        cout << "lbl: " << block2hex(lbl) << endl;
                                        cout << "Correct lbl: " << block2hex(correct_lbl_and0) << endl;
                                        cout << "The other label: " << block2hex(theother_lbl_and0) << endl;
                                    }
                                }

                                /// Evaluate the second AND
                                {
                                    block tweak;
                                    block lbl;

                                    GWI* gin0;
                                    GWI* gin1;
                                    GWI* gout;

                                    int select;

                                    gg_and1 = gc.get_gg(106);
                                    gin0 = gg_and1->m_in0;
                                    gin1 = gg_and1->m_in1;
                                    gout = gg_and1->m_out;

                                    /// Mimic oblivious transfer
                                    gin1->set_lbl(gin1->get_lbl_w_smtc(cval));
                                    /// End mimic oblivious transfer

                                    if (gg_and1->m_is_xor) {

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
                                                          gg_and1->m_egtt->get_row(select),
                                                          AESkey);
                                        }
                                    }
                                    gout->set_lbl(lbl);

                                    /// Check whether lbl is the correct label
                                    EXPECT_EQ(1, block_eq(lbl, correct_lbl_and1));
                                    if (block_eq(lbl, correct_lbl_and1) != 1) {
                                        cout << "lbl: " << block2hex(lbl) << endl;
                                        cout << "Correct lbl: " << block2hex(correct_lbl_and1) << endl;
                                        cout << "The other label: " << block2hex(theother_lbl_and1) << endl;
                                    }
                                }

                                /// Evaluate the OR
                                {
                                    block tweak;
                                    block lbl;

                                    GWI* gin0;
                                    GWI* gin1;
                                    GWI* gout;

                                    int select;

                                    gg_or = gc.get_gg(107);
                                    gin0 = gg_or->m_in0;
                                    gin1 = gg_or->m_in1;
                                    gout = gg_or->m_out;

                                    if (gg_or->m_is_xor) {

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
                                                          gg_or->m_egtt->get_row(select),
                                                          AESkey);
                                        }
                                    }
                                    gout->set_lbl(lbl);

                                    /// Check whether lbl is the correct label
                                    EXPECT_EQ(1, block_eq(lbl, correct_lbl_or));
                                    if (block_eq(lbl, correct_lbl_or) != 1) {
                                        cout << "lbl: " << block2hex(lbl) << endl;
                                        cout << "Correct lbl: " << block2hex(correct_lbl_or) << endl;
                                        cout << "The other label: " << block2hex(theother_lbl_or) << endl;
                                    }
                                }

                                gc = GC();
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
