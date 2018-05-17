/*
 * garbler.cc -- Implementation of garbler
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

#include "garbler.hh"

#include "aes.hh"
#include "ot.hh"
#include "tcp.hh"
#include "util.hh"

#define XOR 6
#define AND 8
#define OR 14

namespace gashgc {

  extern block AESkey;

  const static u32 xor_mnum = xor_magic_num;
  const static u32 nxor_mnum = nonxor_magic_num;

  Garbler::Garbler(u8 port, u8 ot_port, string circ_file_path, string input_file_path) {
    m_port = port;
    m_ot_port = ot_port;
    build_circ(circ_file_path, m_c);
  }

  int Garbler::read_input(string in_file_path) {

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

  int Garbler::garble_circ() {

    block gtt[4];
    block lbl0;
    block lbl1;
    block lbl;

    WI *wi;
    Gate *g;
    GWI *gwi;

    WI *in0;
    WI *in1;
    WI *out;

    GWI *gin0;
    GWI *gin1;
    GWI *gout;

    int func;

    GG *gg;

    u32 id;
    int frst_row_smtc;

    block ZERO = getZEROblock();

    block tweak;

    m_gc.init();

#ifdef GASH_DEBUG

    cout << "R: " << block2hex(gc.R) << endl;
    cout << "R: " << block2hex(gc.R) << endl;

#endif

    for (auto it = m_in_id_set.begin(); it != m_in_id_set.end(); ++it) {

      wi = m_c.get_wireins(*it);

      gwi = new GWI(wi);

      gwi->garble(m_gc.m_R);

      m_gc.add_gwi(gwi);

#ifdef GASH_DEBUG

      cout << "Garbling for wire " << w->get_id() << endl;
      cout << "label0: " << block2hex(gwi->get_lbl0()) << endl;
      cout << "label1: " << block2hex(gwi->get_lbl1()) << endl;

#endif
    }

    for (auto it = m_c.m_gate_map.begin(); it != m_c.m_gate_map.end(); ++it) {

      g = it->second;
      in0 = g->m_in0;
      in1 = g->m_in1;
      out = g->m_out;
      func = g->m_func;

      gin0 = m_gc.get_gwi(in0->get_id());
      gin1 = m_gc.get_gwi(in1->get_id());
      gout = new GWI(out);

#ifdef GASH_DEBUG

      cout << "Garbling gate " << g->get_id() << endl;
      cout << "R: " << block2hex(m_gc.m_R) << endl;

#endif

      /**
       * Garbling an XOR gate
       *
       */
      if (func == XOR) {

        lbl0 = xor_block(gin0->get_lbl0(), gin1->get_lbl0());
        gout->set_lbl0(lbl0);

        lbl1 = xor_block(lbl0, m_gc.m_R);
        gout->set_lbl1(lbl1);
        m_gc.add_gwi(gout);

#ifdef GASH_DEBUG
        // TODO add color system

        {

          cout << "Garble XOR gate" << endl;

          cout << INDENTx1 << "Wire in0 id: " << gin0->get_id() << endl;
          cout << INDENTx1 << "Wire in0 lbl0: " << block2hex(gin0->get_lbl0())
               << endl;
          cout << INDENTx1 << "Wire in0 lbl1: " << block2hex(gin0->get_lbl1())
               << endl;
          cout << INDENTx1
               << "Wire in0 inverted: " << (gin0->m_inv ? "true" : "false") << endl;
          cout << endl;

          cout << INDENTx1 << "Wire in1 id: " << gin1->get_id() << endl;
          cout << INDENTx1 << "Wire in1 lbl0: " << block2hex(gin1->get_lbl0())
               << endl;
          cout << INDENTx1 << "Wire in1 lbl1: " << block2hex(gin1->get_lbl1())
               << endl;
          cout << INDENTx1
               << "Wire in0 inverted: " << (gin0->m_inv ? "true" : "false") << endl;
          cout << endl;

          cout << INDENTx1 << "Wire out id: " << gout->get_id() << endl;
          cout << INDENTx1 << "Wire out lbl0: " << block2hex(gout->get_lbl0())
               << endl;
          cout << INDENTx1 << "Wire out lbl1: " << block2hex(gout->get_lbl1())
               << endl;
          cout << endl;

        }

#endif

        gg = new GG(1, gin0, gin1, gout);

      } else {

        /**
         * Garbling an AND/OR gate
         *
         */

        // Construct the tweak
        tweak = new_tweak(g->get_id());

        // Get the semantic of the first row of egtt
        frst_row_smtc =
          eval_bgate(gin0->get_smtc_w_lsb(0), gin1->get_smtc_w_lsb(1), func);

        // Encrypt the label
        lbl = encrypt(gin0->get_lbl_w_lsb(0), gin1->get_lbl_w_lsb(0), tweak, ZERO,
                      AESkey);

        gout->set_lbl_w_smtc(lbl, frst_row_smtc);
        gout->set_lbl_w_smtc(xor_block(lbl, m_gc.m_R), frst_row_smtc ^ 1);

        lbl0 = gout->get_lbl_w_smtc(0);
        lbl1 = gout->get_lbl_w_smtc(1);

        // Add output wire to circuit
        m_gc.add_gwi(gout);

#ifdef GASH_DEBUG

        {

          cout << "Garbling an AND/OR gate" << endl;

          cout << INDENTx1 << "Wire in0 id: " << gin0->get_id() << endl;
          cout << INDENTx1 << "Wire in0 lbl0: " << block2hex(gin0->get_lbl0())
               << endl;
          cout << INDENTx1 << "Wire in0 lbl1: " << block2hex(gin0->get_lbl1())
               << endl;
          cout << endl;

          cout << INDENTx1 << "Wire in1 id: " << gin1->get_id() << endl;
          cout << INDENTx1 << "Wire in1 lbl0: " << block2hex(gin1->get_lbl0())
               << endl;
          cout << INDENTx1 << "Wire in1 lbl1: " << block2hex(gin1->get_lbl1())
               << endl;
          cout << endl;

          cout << INDENTx1 << "Wire out id: " << gout->get_id() << endl;
          cout << INDENTx1 << "Wire out lbl0: " << block2hex(gout->get_lbl0())
               << endl;
          cout << INDENTx1 << "Wire out lbl1: " << block2hex(gout->get_lbl1())
               << endl;
          cout << endl;
        }

#endif

        // Write label to encrypted garbled truth table
        gtt[1] =
          encrypt(gin0->get_lbl_w_lsb(1), gin1->get_lbl_w_lsb(0), tweak,
                  gout->get_lbl_w_smtc(eval_bgate(
                                                  gin0->get_smtc_w_lsb(1), gin1->get_smtc_w_lsb(0), func)),
                  AESkey);

        gtt[2] =
          encrypt(gin0->get_lbl_w_lsb(0), gin1->get_lbl_w_lsb(1), tweak,
                  gout->get_lbl_w_smtc(eval_bgate(
                                                  gin0->get_smtc_w_lsb(0), gin1->get_smtc_w_lsb(1), func)),
                  AESkey);

        gtt[3] =
          encrypt(gin0->get_lbl_w_lsb(1), gin1->get_lbl_w_lsb(1), tweak,
                  gout->get_lbl_w_smtc(eval_bgate(
                                                  gin0->get_smtc_w_lsb(1), gin1->get_smtc_w_lsb(1), func)),
                  AESkey);

#ifdef GASH_DEBUG

        {

          cout << "Writing to EGTT" << endl;

          cout << INDENTx1 << "gtt[0]: " << 0 << endl;
          cout << INDENTx1 << "gtt[1]: " << block2hex(gtt[1]) << endl;
          cout << INDENTx1 << "gtt[2]: " << block2hex(gtt[2]) << endl;
          cout << INDENTx1 << "gtt[3]: " << block2hex(gtt[3]) << endl;
          cout << endl;
        }

#endif

        // Create the garbled gate with garbled truth table
        EGTT *egtt = new EGTT(gtt[1], gtt[2], gtt[3]);
        gg = new GG(0, gin0, gin1, gout, egtt);
      }

      // Add garbled gate to the garbled circuit
      REQUIRE_GOOD_STATUS(m_gc.add_gg(gg));

#ifdef GASH_DEBUG

      {

        cout << "Adding label pair" << endl;
        cout << INDENTx1 << "'Wire id: " << gout->get_id() << endl;
        cout << INDENTx1 << "Inverted: " << gout->m_inv ? "true"
          : "false" << endl;
        cout << INDENTx1 << "label0: " << block2hex(gout->get_lbl0()) << endl;
        cout << INDENTx1 << "label1: " << block2hex(gout->get_lbl1()) << endl;
        cout << endl;

      }

#endif

    }

    return 0;
  }

  int Garbler::init_connection() {

    return tcp_server_init(m_port, m_listen_sock, m_peer_sock);

  }

  int Garbler::send_egtt() {

    u32          id;
    GG*          gg;
    u32          size;
    block        row1;
    block        row2;
    block        row3;

    size = m_gc.m_gg_map.size();
    REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&size, sizeof(u32)));

    for (auto it = m_gc.m_gg_map.begin(); it != m_gc.m_gg_map.end(); ++it) {

      id = it->first;
      gg = it->second;

      REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&id, sizeof(u32)));

      // If it is an xor gate, just tell the peer by sending this magic number
      if (gg->m_is_xor) {

        REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&xor_mnum, sizeof(u32)));

      } else {
        // If not, send rows in the EGTT

        REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&nxor_mnum, sizeof(u32)));

        GASSERT(gg->m_egtt != NULL);
        row1 = gg->m_egtt->get_row(1);
        row2 = gg->m_egtt->get_row(2);
        row3 = gg->m_egtt->get_row(3);

        REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&row1, LABELSIZE));
        REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&row2, LABELSIZE));
        REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&row3, LABELSIZE));

      }

    }

    return 0;
  }

  int Garbler::send_self_lbls() {

    u32 size;
    u32 id;
    int val;
    block lbl;

    size = m_self_in_id_set.size();
    REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&size, sizeof(u32)));

    for (auto it = m_self_in_id_set.begin(); it != m_self_in_id_set.end(); ++it) {

      id = *it;
      auto val_it = m_in_val_map.find(id);
      GASSERT(val_it != m_in_val_map.end());
      val = val_it->second;
      GASSERT(val == 0 || val == 1);

      REQUIRE_GOOD_STATUS(m_gc.get_lbl(id, val, lbl));

      REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&lbl, LABELSIZE));
    }

    return 0;
  }

#ifdef GASH_NO_OT

  /**
   * If OT is disabled, send all peer's label to peer
   *
   * @return
   */
  int Garbler::send_peer_lbls() {

    u32         id;
    u32         size;
    block       lbl0;
    block       lbl1;

    size = m_peer_in_id_set.size();
    REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char *)&size, sizeof(u32)));

    for (auto it = m_peer_in_id_set.begin(); it != m_peer_in_id_set.end(); ++it) {

      id = *it;
      REQUIRE_GOOD_STATUS(m_gc.get_lbl(id, 0, lbl0));
      REQUIRE_GOOD_STATUS(m_gc.get_lbl(id, 1, lbl1));

      REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char *)&id, sizeof(u32)));
      REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char *)&lbl0, LABELSIZE));
      REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char *)&lbl1, LABELSIZE));
    }

    return 0;
  }

#else

  int Garbler::send_peer_lbls() {

    u32          id;
    u32          size;
    block        lbl0;
    block        lbl1;
    LabelVec     lbl0vec;
    LabelVec     lbl1vec;

    size = m_gc.m_peer_in_id_set.size();
    REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&size, sizeof(u32)));

    for (auto it = m_peer_in_id_set.begin(); it != m_peer_in_id_set.end(); ++it) {

      id = *it;

      REQUIRE_GOOD_STATUS(m_gc.get_lbl(id, 0, lbl0));
      REQUIRE_GOOD_STATUS(m_gc.get_lbl(id, 1, lbl1));

      lbl0vec.emplace_back(lbl0);
      lbl1vec.emplace_back(lbl1);

    }

    // Call OTSend
    return OTSend(m_peer_ip, m_peer_ot_port, lbl0vec, lbl1vec);
  }

#endif

  int Garbler::send_output_map() {

    u32 id;
    u32 size;
    int val;
    block lbl0;
    block lbl1;

    size = m_out_id_set.size();
    REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char*)&size, sizeof(u32)));

    for (auto it = m_out_id_set.begin(); it != m_out_id_set.end(); ++it) {

      id = *it;
      REQUIRE_GOOD_STATUS(m_gc.get_lbl(id, 0, lbl0));
      REQUIRE_GOOD_STATUS(m_gc.get_lbl(id, 1, lbl1));

      REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char *)&id, sizeof(u32)));
      REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char *)&lbl0, LABELSIZE));
      REQUIRE_GOOD_STATUS(tcp_send_bytes(m_peer_sock, (char *)&lbl1, LABELSIZE));

    }

    return 0;
  }

  int Garbler::recv_output() {

    u32 id;
    u32 size;
    int val;

    REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&size, sizeof(u32)));
    if (size != m_out_id_set.size()) {
      WARNING("Output size inconsistent, expecting " << m_out_id_set.size() << ", getting " << size);
    }

#ifdef GASH_DEBUG

    cout << "Receive output from evaluator :)" << endl;

#endif

    for (u32 i = 0; i < size; ++i) {

      REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&id, sizeof(u32)));
      REQUIRE_GOOD_STATUS(tcp_recv_bytes(m_peer_sock, (char*)&val, sizeof(u32)));

      m_out_val_map.emplace(id, val);

#ifdef GASH_DEBUG

      cout << "Wire id: " << id << endl;
      cout << "Value: " << val << endl;
      cout << endl;

#endif

    }

    return 0;
  }

  Garbler::~Garbler() {

    for (auto it = m_gc.m_gg_map.begin(); it != m_gc.m_gg_map.end(); ++it) {
      delete it->second;
    }

  }

} // namespace gashgc
