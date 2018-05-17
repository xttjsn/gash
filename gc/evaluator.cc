/*
 * evaluator.cc -- Implementation of evaluator
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

#include "evaluator.hh"

#include "aes.hh"
#include "ot.hh"
#include "tcp.hh"
#include "util.hh"

namespace gashgc {

  extern block AESkey;

  Evaluator::Evaluator(u8 port, u8 ot_port, string circ_file_path, string input_file_path) {
    m_port = port;
    m_ot_port = ot_port;

    if (build_circ(circ_file_path, m_c) < 0) {
      FATAL("Failed to build circuit");
    }

    if (build_garbled_circuit() < 0)  {
      FATAL("Failed to build garbled circuit");
    }
  }

  int Evaluator::build_garbled_circuit() {

    WI*          in0;
    WI*          in1;
    WI*          out;
    Gate*        g;

    GWI*         gin0;
    GWI*         gin1;
    GWI*         gout;
    GG*        gg;

    for (auto it = m_c.m_gate_map.begin(); it != m_c.m_gate_map.end(); ++it) {

      g   = it->second;
      in0 = g->m_in0;
      in1 = g->m_in1;
      out = g->m_out;

      gin0 = new GWI(in0);
      gin1 = new GWI(in1);
      gout = new GWI(out);
      gg   = new GG(g->m_func == funcXOR, gin0, gin1, gout);

      REQUIRE_GOOD_STATUS(m_gc.add_gwi(gin0));
      REQUIRE_GOOD_STATUS(m_gc.add_gwi(gin1));
      REQUIRE_GOOD_STATUS(m_gc.add_gwi(gout));
      REQUIRE_GOOD_STATUS(m_gc.add_gg(gg));

    }

    return 0;

  }

  int Evaluator::read_input(string in_file_path) {

    ifstream file(in_file_path);
    if (!file.is_open()) {
      FATAL("Unable to open input data file " << in_file_path);
    }

    string line;
    u32 linum;
    u32 size;
    u32 id;
    int val;

    while (getline(file, line)) {

      linum++;

      // Split the line by whitespace
      vector<string> items;
      const char* delim = " ";
      split(line, delim, items);

      // Prologue
      if (items[0].compare("input") == 0) {

        size = stoi(items[1]);
        m_n_self_in = size;
        m_n_peer_in = m_c.m_nin - size;

        continue;
      }

      // Data
      if (items.size() == 2) {

        id = stoi(items[0]) / 2;
        val = stoi(items[1]);
        GASSERT(val == 0 || val == 1);

        // Check duplication
        if (m_in_val_map.find(id) != m_in_val_map.end()) {
          FATAL("Line " << linum << "-Duplicate id detected while reading data input: " << id);
        }

        // Insert the id-value pair
        m_in_val_map.emplace(id, val);

        // Insert the id to the set containing all self input wire ids
        m_self_in_id_set.emplace(id);
      }

      // Others
      FATAL("Line " << linum << "-Invalid line while reading data input :" << linum);
    }

    return 0;
  }

  int Evaluator::init_connection() {

    return tcp_client_init(m_peer_ip, m_port, m_peer_sock);

  }

  int Evaluator::evaluate_circ() {

    block         tweak;
    block         lbl;
    block         ZERO = getZEROblock();

    GWI           *gin0;
    GWI           *gin1;
    GWI           *gout;
    GG            *gg;

    u32           id;
    int           select;


    for (auto it = m_gc.m_gg_map.begin(); it != m_gc.m_gg_map.end(); ++it) {

      id = it->first;
      gg = m_gc.get_gg(id);

      if (!gg) {
        WARNING("Cannot find garbled gate for id:" << id);
        return -G_ENOENT;
      }

      gin0 = gg->m_in0;
      gin1 = gg->m_in1;
      gout = gg->m_out;

      // XOR gate
      if (gg->m_is_xor) {

        lbl = xor_block(gin0->get_lbl(), gin1->get_lbl());

      } else {
        // Non-XOR gate

        select = get_lsb(gin0->get_lbl()) + (get_lsb(gin1->get_lbl()) << 1);
        tweak  = new_tweak(gg->get_id());

        if (select == 0) {

          // Use the same encryption as in garbling to get the label
          lbl = encrypt(gin0->get_lbl(), gin1->get_lbl(), tweak, ZERO, AESkey);

        } else {

          lbl = decrypt(gin0->get_lbl(),
                        gin1->get_lbl(),
                        tweak,
                        gg->m_egtt->get_row(select),
                        AESkey);

        }

      }

      gout->set_lbl(lbl);

    }

    return 0;
  }

  int Evaluator::recv_egtt() {

    u32           id;
    GG*           gg;
    u32           size;
    u32           magic_num;
    block         row1;
    block         row2;
    block         row3;

    REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&size, sizeof(u32)));
    GASSERT(size == m_gc.m_ngate);  // Assert that peer is sending the same number of gates

    for (u32 i = 0; i < size; ++i) {

      REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&id, sizeof(u32)));
      REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&magic_num, sizeof(u32)));

      // XOR gate, skip it since we need no egtt for xor gates
      if (magic_num == xor_magic_num) {

        continue;

      } else if (magic_num == nonxor_magic_num) {
        // Non-xor gates, receive egtt

        REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&row1, LABELSIZE));
        REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&row2, LABELSIZE));
        REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&row3, LABELSIZE));

        gg = m_gc.get_gg(id);
        GASSERT(gg != NULL);

        gg->m_egtt = new EGTT(row1, row2, row3);

      } else {

        WARNING("Invalid magic number: " << magic_num);
        return -G_EINVAL;

      }

    }

    return 0;
  }

#ifdef GASH_NO_OT

  int Evaluator::recv_self_lbls() {

    u32             id;
    u32             size;
    GWI*            gw;
    block           lbl0;
    block           lbl1;
    block           lbl;
    int             val;

    REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&size, sizeof(u32)));

    for (u32 i = 0; i < size; ++i) {

      REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&id, sizeof(u32)));
      REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&lbl0, LABELSIZE));
      REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&lbl1, LABELSIZE));

      gw = m_gc.get_gwi(id);
      if (!gw) {
        WARNING("Cannot find value for wire id: " << id);
        return -G_ENOENT;
      }

      val = m_in_val_map.find(id)->second;

      lbl = val == 0 ? lbl0 : lbl1;

      gw->set_lbl(lbl);
    }

    return 0;
  }

#else

  int Evaluator::recv_self_lbls() {

    LabelVec        lblvec;
    IdValueMap      idvalmap;
    int             val;

    REQUIRE_GOOD_STATUS(OTRecv(m_peer_ip, m_peer_ot_port, m_id_val_map, lbl1vec));

  }

#endif

  int Evaluator::recv_output_map() {

    u32            id;
    u32            size;
    GWI*           gw;
    block          lbl0;
    block          lbl1;

    REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&size, sizeof(u32)));

    for (u32 i = 0; i < size; ++i) {

      REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char *)&id, sizeof(u32)));
      REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char *)&lbl0, LABELSIZE));
      REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char *)&lbl1, LABELSIZE));

      if (m_out_id_set.find(id) == m_out_id_set.end()) {
        WARNING("Cannot find id in output id set:" << id);
        return -G_ENOENT;
      }

      gw = m_gc.get_gwi(id);
      if (!gw) {
        WARNING("Cannot find value for wire id: " << id);
        return -G_ENOENT;
      }

      gw->set_lbl0(lbl0);
      gw->set_lbl1(lbl1);

    }

    return 0;

  }

  int Evaluator::recover_output() {

    u32           id;
    GWI*          gw;
    int           val;

    for (auto it = m_out_id_set.begin(); it != m_out_id_set.end(); ++it) {

      id = *it;
      gw = m_gc.get_gwi(id);

      val = gw->recover_smtc();
      if (val < 0) {
        return val;
      }

      m_out_val_map.emplace(id, val);
    }

    return 0;
  }

  int Evaluator::send_output() {

    u32          id;
    u32          size;
    int          val;

    size = m_out_val_map.size();
    REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&size, sizeof(u32)));

    for (auto it = m_out_val_map.begin(); it != m_out_val_map.end(); ++it) {

      id = it->first;
      val = it->second;

      REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&id, sizeof(u32)));
      REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&val, sizeof(int)));

    }

    return 0;
  }

}
