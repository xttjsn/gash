/*
 * circuit.cc -- Circuit implementation
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

#include "circuit.h"

namespace gashgc {

  /**
   * Circuit
   *
   */

  Gate* Circuit::get_gate(u32 i) {
    return m_gate_map[i];
  }

  WireInstance* Circuit::get_wireins(u32 i) {
    return m_wireins_map[i];
  }

  int Circuit::add_wireins(WireInstance* w) {
    if (m_wireins_map.find(w->get_id()) != m_wireins_map.end()) {
      return -EEXIST;
    }
    m_wireins_map.insert(make_pair(w->get_id(), w));
    return 0;
  }

  int Circuit::add_gate(Gate* g) {
    if (has_gate(g->get_id())) {
      return -EEXIST;
    }
    m_gate_map.insert(make_pair(g->get_id(), g));
    return 0;
  }

  bool Circuit::has_gate(u32 id) {
    return m_gate_map.find(id) != m_gate_map.end();
  }

}
