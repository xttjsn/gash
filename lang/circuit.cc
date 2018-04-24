/*
 * circuit.cc -- Implementation of circuit related classes
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

namespace gashlang {

  static u32 wid = 0;

  /**
   * Several helper functions for Wire
   *
   */

  inline bool is_odd(int n) {
    return !(n % 2 == 0);
  }

  inline int evenify(int n) {
    return is_odd(n) ? n-1 : n;
  }

  /**
   * Wire related functions
   */
  void Wire::invert() {
    if (is_odd(this->m_id)) {
      this->m_id--;
    } else {
      this->m_id++;
    }
  }

  void Wire::set_id_even() {
    if (is_odd(this->m_id)) {
      this->m_id--;
    }
  }

  void Wire::set_id_odd() {
    if (!is_odd(this->m_id)) {
      this->m_id++;
    }
  }

  void Wire::invert_from(Wire* w) {
    if (is_odd(w->m_id)) {
      this->set_id_even();
    } else {
      this->set_id_odd();
    }
  }

  void wput(Wire* w) {
    if (--w->m_refcount == 0) {
      delete w;
    }
  }

  void wref(Wire* w) {
    ++w->m_refcount;
  }

  /**
   * Bunble related functions
   */
  Bundle::Bundle(vector<Wire*>& wires) {
    m_wires = wires;
  }

  Bundle::Bundle(Wire* w) {
    m_wires.push_back(w);
  }

  Bundle::Bundle(u32 len) {
    for (u32 i = 0; i < len; i++) {
      Wire* w = nextwire();
      w->m_v = 0;  // Default value is 0.
      m_wires.push_back(w);
    }
  }

  void Bundle::add(Wire* w) {
    m_wires.push_back(w);
    m_wires_map.insert(make_pair(w->m_id, w));
  }

  bool Bundle::hasWire(u32 i) {
    return m_wires_map.find(i) != m_wires_map.end();
  }

  Wire* Bundle::getWire(u32 i) {
    if (m_wires_map.find(i) == m_wires_map.end()) {
      return NULL;
    }
    return m_wires_map.find(i)->second;
  }

  Wire* Bundle::operator[](u32 i) {
    return m_wires[i];
  }

  u32 Bundle::size() {
    return m_wires.size();
  }

  int Bundle::copyfrom(Bundle& src, u32 start, u32 srcstart, u32 size) {
    if (size > this->size() - start) {
      std::cerr << "This bundle has not enough room to copy from another bundle." << std::endl;
      return 1;
    }
    if (src.size() - srcstart < size) {
      std::cerr << "The source bundle has not enough wires." << std::endl;
      return 1;
    }
    for (u32 i = 0; i < size; ++i) {
      this->m_wires[i + start] = src[i + srcstart];
    }
    return 0;
  }

  void Bundle::clear() {
    while (this->m_wires.size() > 0) {
      Wire* w = this->m_wires.back();
      this->m_wires.pop_back();
      this->m_wires_map.erase(w->m_id);
      wput(w);
    }
  }

  void Bundle::clear_val() {
    for (auto it = this->m_wires.begin(); it != this->m_wires.end(); ++it) {
      (*it)->m_v = -1;
    }
  }

  void num2bundle_n(i64 v, Bundle& bret, u32 n) {
    u32 i = 0;
    bret.clear();
    while (i < 64 && i < n) {
      bret.add(getbit(v, i) == 1 ? W_ONE : W_ZERO);
    }
  }

  void num2bundle(i64 v, Bundle& bret) {
    num2bundle_n(v, bret, 64);
  }

  /**
   * Circuit implementation
   */

  Circuit::Circuit(ostream& circ_stream, ostream& data_stream) {
    m_circ_stream = &circ_stream;
    m_data_stream = &data_stream;
  }

  void Circuit::write() {
    m_prologue.emit(*m_circ_stream);
    m_in.emit(*m_circ_stream);
    m_out.emit(*m_circ_stream);
    m_gates.emit(*m_circ_stream);
    write_input();
  }

  void Circuit::write_input() {
    u32 w_id;
    u32 bit;
    ostream& stream = *m_data_stream;

    stream << "input " << m_input_data.size() << endl;
    for (u32 i = 0; i < m_input_data.size(); ++i) {
      w_id = m_input_data.find(i)->first;
      bit = m_input_data.find(i)->second;
      stream << evenify(w_id) << ' ' << bit << endl;
    }
  }

  void Circuit::add_input_values(Bundle& bundle) {
    u32 wire_id;
    u32 wire_dup_id;
    u32 bitval;

    for (u32 i = 0; i < bundle.size(); ++i) {
      wire_id = bundle[i]->m_id;
      bitval = bundle[i]->m_v;
      m_input_data.insert(make_pair(wire_id, bitval));

      if (m_input_duplicates.find(wire_id) != m_input_duplicates.end()) {
        // Found input duplicate
        wire_dup_id = m_input_duplicates.find(wire_id)->second;
        m_input_data.insert(make_pair(wire_dup_id, 1 ^ bitval));
      }
    }

    // Check consistency against m_in.
    WireIdValueMap wire_existence_map;
    for (u32 i = 0; i < m_in.size(); ++i) {
      wire_existence_map.insert(make_pair(m_in[i]->m_id, 1));
    }

    for (u32 i = 0; i < bundle.size(); ++i) {
      if (wire_existence_map.find(bundle[i]->m_id) == wire_existence_map.end())
        throw std::runtime_error("Wire exists in input value bundle, but cannot find it in m_in, probably m_in hasn't been properly setup.");
      if (m_input_duplicates.find(wire_id) != m_input_duplicates.end()) {
        // Found input duplicate
        wire_dup_id = m_input_duplicates.find(wire_id)->second;
        if (wire_existence_map.find(wire_dup_id) == wire_existence_map.end())
          throw std::runtime_error("Wire's invert exists in input value bundle, but cannot find it in m_in, probably m_in hasn't been properly setup.");
      }
    }
  }

  void Circuit::add_input_wire(Wire* w) {
    m_in.add(w);
    m_prologue.numIN++;
  }

  bool Circuit::has_input_duplicate(Wire* w) {
    return m_input_duplicates.find(w->m_id) != m_input_duplicates.end();
  }

  bool Circuit::is_input_wire(Wire* w) {
    Wire* w_another;
    for (u32 i = 0; i < m_in.size(); ++i) {
      w_another = m_in[i];
      // Do some consistency check along the way
      if (w->m_id == w_another->m_id) {
        WARNING("Two wire pointers point to the same wire instance. ID:"  << w->m_id);
        if (w->m_v != w->m_v) {
          FATAL("Two wire pointers point to the same wire instance but have different value. ID:" << w->m_id);
        }
        return true;
      }
    }
    return false;
  }

  void Circuit::set_input_invert_duplicate(Wire* w, Wire* w_dup) {
    u32 id_original = w->m_id;
    u32 id_duplicate = w_dup->m_id;
    if (m_input_duplicates.find(id_original) != m_input_duplicates.end()) {
      FATAL("Input wire with id " << id_original << " is already_duplicated.");
    }
    m_input_duplicates.insert(make_pair(id_original, id_duplicate));
    // m_input_duplicates.insert(make_pair(id_duplicate, id_original));
    set_invert_wire(w, w_dup);
    set_invert_wire(w_dup, w);
    add_input_wire(w_dup);
  }

  bool Circuit::has_invert_wire(Wire* w) {
    return m_wire_inverts.find(w->m_id) != m_wire_inverts.end();
  }

  Wire* Circuit::get_invert_wire(Wire* w) {
    Wire* w_inv;
    if (has_invert_wire(w))
      w_inv = m_wires.find(m_wire_inverts.find(w->m_id)->second)->second;
    return w_inv;
  }

  int Circuit::set_invert_wire(Wire* w, Wire* w_inv) {
    if (has_invert_wire(w)) {
      FATAL("Wire " << w->m_id << "already has inverted wire.");
    }
    if (has_invert_wire(w_inv)) {
      FATAL("Wire " << w_inv->m_id << "already has inverted wire.");
    }
    m_wire_inverts.insert(make_pair(w->m_id, w_inv->m_id));
  }


} // gashlang
