/*
 * garbled_circuit.cc -- Implementation of garbled circuit
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

#include "garbled_circuit.h"

namespace gashgc {

  GG* GC::get_gg(u32 i) {
    return m_gg_map.at(i);
  }

  int GC::add_gg(GG* gate) {
    if (has_gg(gate->m_id)) {
      return -EEXIST;
    }
    m_gg_map.insert(std::make_pair(gate->m_id, gate));
    return 0;
  }

  bool GC::has_gg(u32 id) {
    return m_gg_map.find(id) != m_gg_map.end();
  }

  int GC::add_gg(Circuit &circ, u32 id, block row1, block row2, block row3) {

    if (has_gg(id)) {
      return -EEXIST;
    }

    Gate* g = circ.get_gate(id);

    u32 id_0   = g->m_in0->get_id();

    u32 id_1   = g->m_in1->get_id();

    u32 id_out = g->m_out->get_id();

    GWI* in0;

    GWI* in1;

    GWI* out;

    int status;

    if (!has_gwi(id_0)) {

      in0 = new GWI(g->m_in0);

      if ( (status = this->add_gwi(in0)) < 0) {

        return status;

      }

    }
    else {

      in0 = m_gwi_map.find(id_0)->second;

    }

    if (!has_gwi(id_1)) {

      in1 = new GWI(g->m_in1);

      if ( (status = this->add_gwi(in1)) < 0) {

        return status;

      }

    }
    else {

      in1 = m_gwi_map.find(id_1)->second;

    }

    if (!has_gwi(id_out)) {

      out = new GWI(g->m_out);

      if ( (status = this->add_gwi(out)) < 0) {

        return status;

      }

    }
    else {

      out = m_gwi_map.find(id_out)->second;

    }

    GG* gg = new GG(false, in0, in1, out);

    EGTT* table = new EGTT(row1, row2, row3);

    gg->m_egtt = table;

    this->add_gg(gg);

  }

  GWI* GC::get_gwi(u32 i) {
    return m_gwi_map.at(i);
  }

  int GC::add_gwi(GWI* w) {
    if (has_gwi(w->get_id())) {
      return -EEXIST;
    }
    m_gwi_map.insert(make_pair(w->get_id(), w));
  }

  int GC::set_gwl(u32 id, block label) {
    if (!has_gwi(id)) {
      return -ENOENT;
    }
    m_gwi_map.find(id)->second->set_lbl(label);
    return 0;
  }

  void GC::init() {
    m_R = random_block();
  }

  void GC::debug_report_garbler() {

    GWI *in0, *in1, *out;
    GG* g;

    std::cout << "R: " << block2dec(this->m_R) << endl;

    for (auto it = m_gg_map.begin(); it != m_gg_map.end(); ++it) {

      g = it->second;

      if (!g) {
        FATAL("GG with output wire id" << g->m_out->get_id() << "is null");
      }

      in0 = g->m_in0;
      in1 = g->m_in1;
      out = g->m_out;

      cout << "Gate " << g->get_id() << ":" << endl;

      cout << "Is XOR= " << g->m_is_xor << endl;

      cout << TABx1 << "in0=" << in0->get_id() << ":" << endl << endl;

      cout << TABx2 << "label0=" << block2dec(in0->get_lbl0()) << endl;
      cout << TABx2 << "label1=" << block2dec(in0->get_lbl1()) << endl;

      cout << TABx1 << "in1=" << in1->get_id() << ":" << endl;

      cout << TABx2 << "label0=" << block2dec(in1->get_lbl0()) << endl;
      cout << TABx2 << "label1=" << block2dec(in1->get_lbl1()) << endl;

      cout << TABx1 << "out=" << out->get_id() << ":" << endl;

      cout << TABx2 << "label0=" << block2dec(out->get_lbl0()) << endl;
      cout << TABx2 << "label1=" << block2dec(out->get_lbl1()) << endl;

      cout << TABx1 << "table:" << endl;

      cout << TABx2 << "row1=" << block2dec(g->m_egtt->get_row(1)) << endl;
      cout << TABx2 << "row2=" << block2dec(g->m_egtt->get_row(2)) << endl;
      cout << TABx2 << "row3=" << block2dec(g->m_egtt->get_row(3)) << endl;
    }

  }

  void GC::debug_report_evaluator() {

    GG* g;

    for (auto it = m_gg_map.begin(); it != this->mm_gg_map.end(); ++it) {

      g = it->second;

      cout << "Gate " << g->get_id() << ":" << endl;

      cout << TABx1 << "isXOR= " << g->m_is_xor << endl;

      if (!g->m_is_xor) {

        cout << TABx2 << "table: " << endl;
        cout << TABx2 << "row1=" << block2dec(g->m_egtt->get_row(1)) << endl;
        cout << TABx2 << "row2=" << block2dec(g->m_egtt->get_row(2)) << endl;
        cout << TABx2 << "row3=" << block2dec(g->m_egtt->get_row(3)) << endl;
      }
    }
  }


  GC::~GC() {

    for (auto it = m_gwi_map.begin(); it != m_gwi_map.end(); ++it) {
      delete it->second;
    }

    for (auto it = m_gg_map.begin(); it != this->m_gg_map.end(); ++it) {
      delete it->second;
    }

  }
}
