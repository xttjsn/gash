/*
 * ot.cc -- Unit testing for oblivious transfer
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

#define nlbls      1000

typedef vector<block> BlockVec;
typedef map<u32, int> IdSelMap;
typedef map<u32, block> IdBlockMap;

u16 s_port = 49876;        // Sender port
string s_ip ="127.0.0.1";

TEST_F(OTTest, SendRcv) {

  BlockVec    lbl0vec;
  BlockVec    lbl1vec;
  IdSelMap    selmap;
  IdBlockMap  res_idlbl_map;
  int         val;
  block       lbl;
  block       correct_lbl;

  for (int i = 0; i < nlbls; ++i) {
    lbl0vec.emplace_back(random_block());
    lbl1vec.emplace_back(random_block());
    selmap.emplace(i, rand() % 2);          // Random selection between 0 and 1
  }

  // Sender is the server
  if (fork() == 0) {
    // Child plays receiver
    sleep(2.0);
    OTParty otp;
    EXPECT_EQ(0, otp.OTRecv(s_ip, s_port, selmap, res_idlbl_map));

    // Verify that the received labels are correct
    for (auto it = selmap.begin(); it != selmap.end(); ++it) {
      int id = it->first;
      val = it->second;
      correct_lbl = val == 0 ? lbl0vec[id] : lbl1vec[id];

      GASSERT(res_idlbl_map.find(id) != res_idlbl_map.end());
      lbl = res_idlbl_map.find(id)->second;
      EXPECT_EQ(1, block_eq(lbl, correct_lbl));
    }

  } else{
    // Parent plays sender
    OTParty otp;
    EXPECT_EQ(0, otp.OTSend(s_ip, s_port, lbl0vec, lbl1vec));

  }

}

int main(int argc, char* argv[])
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
