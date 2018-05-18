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

#define NTEST      20

namespace gashgc {
  extern block AESkey;
}

using gashgc::AESkey;
using gashgc::random_block;
using gashgc::get_lsb;
using gashgc::set_lsb;
using gashgc::new_tweak;
using gashgc::encrypt;
using gashgc::decrypt;
using gashgc::block_eq;
using gashgc::block2hex;
using gashgc::aes_encrypt128;
using gashgc::aes_decrypt128;


TEST_F(GRBLTest, REndsWith1) {

  for (int i = 0; i < NTEST; ++i) {
    m_gc.init();
    EXPECT_EQ(1, get_lsb(m_gc.m_R));
  }

}

TEST_F(GRBLTest, TwoLablesHaveOppositeLSB) {
  WI*  wi  = new WI(100, 0);
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

TEST_F(GRBLTest, CorrectDecryption) {

  block a = random_block();
  block b = random_block();
  block c = random_block();
  block cipher;
  block dec;
  block tweak;

  for (int i = 0; i < NTEST; ++i) {
    tweak  = new_tweak(i);
    cipher = encrypt(a, b, tweak, c, AESkey);
    dec    = decrypt(a, b, tweak, cipher, AESkey);
    EXPECT_EQ(1, block_eq(c, dec));
  }

}


// TEST_F(GRBLTest, CorrectEGTTDecryption) {


// }

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
