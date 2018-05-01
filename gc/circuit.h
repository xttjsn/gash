/*
 * circuit.h -- GC circuit related class
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

#ifndef GASH_GC_CIRCUIT_H
#define GASH_GC_CIRCUIT_H

#include "../include/common.h"

namespace gashgc {

  class Wire;
  class WireInstance;
  class Gate;
  class Circuit;

  typedef map<u32, WireInstance*> IdWireinsMap;
  typedef map<u32, Gate*> IdGateMap;
  typedef set<u32> IdSet;
  typedef vector<u32> IdVec;
  typedef map<u32, bool> IdBoolMap;


  /**
   * Circuit
   *
   */
  class Circuit {
  public:

    u32 m_nin;

    u32 m_nout; // n: number of inputs, m: number of outputs, q: number of gates

    u32 m_ngate;

    IdWireinsMap m_wireins_map;

    IdGateMap m_gate_map;

    IdSet m_out_id_set;

    IdVec m_in_id_vec;

    IdVec m_out_id_vec;

    IdBoolMap m_out_const_map;

    /**
     * Get wire instance with id
     *
     * @param id
     *
     * @return
     */
    WireInstance* get_wireins(u32 id);

    /**
     * Get gate with id
     *
     * @param i
     *
     * @return
     */
    Gate* get_gate(u32 i);

    /**
     * Add a wire instance to circuit
     *
     * @param w
     *
     * @return
     */
    int add_wireins(WireInstance* w);

    /**
     * Add gate to circuit
     *
     * @param g
     */
    int add_gate(Gate* g);

    /**
     * Return 1 if circuit has gate
     *
     * @param id
     *
     * @return
     */
    bool has_gate(u32 id);

  };


  /**
   * Wire
   *
   */
  class Wire {
  public:

    u32 m_id;

    int m_val = -1;              // Only use for debugging

    Wire(u32 id) : m_id(id) {}

    Wire(u32 id, int val) : m_id(id), m_val(val) {}

  };

  /**
   * WireInstance
   *
   */
  class WireInstance {
  public:

    Wire* m_wire = NULL;
    bool m_inv = false;
    u32 m_inv_times = 0;

    WireInstance() {}

    WireInstance(u32 id, bool inv) {
      m_wire = new Wire(id);
      m_inv = inv;
    }

    // WireInstance(Wire & wire) : wire_ptr_(&wire), inverted_(false), inverted_set_times_(0) {}
    // WireInstance(Wire* wire_ptr) : wire_ptr_(wire_ptr), inverted_(false), inverted_set_times_(0) {}
    // WireInstance(Wire & wire, bool inverted) : wire_ptr_(&wire), inverted_(inverted), inverted_set_times_(0) {}
    // WireInstance(Wire* wire_ptr, bool inverted) : wire_ptr_(wire_ptr), inverted_(inverted), inverted_set_times_(0) {}

    // WireInstance(ulong id, bool inverted) {
    //   GASH_REQUIRE_GOOD_ALLOC(this->wire_ptr_ = new Wire);
    //   set_id(id);
    //   inverted_set_times_ = 0;
    //   set_inverted(inverted);
    // }

    /**
     * Get id
     *
     *
     * @return
     */
    u32 get_id() {
      return m_wire->m_id;
    }

    /**
     * Set id
     *
     * @param id
     *
     * @return
     */
    u32 set_id(u32 id) {
      m_wire->m_id = id;
    }

    /**
     * Get invertibility
     *
     *
     * @return
     */
    bool get_inv() {
      return m_inv;
    }

    /**
     * Set invertibility
     *
     * @param inv
     */
    void set_inv(bool inv) {
      if (++m_inv_times > 2) {
        FATAL("A Wireinstance's can only be invert twice, firstly as an output  \
        wire (during initialzation), once as an input wire (during              \
        reference).");
      }
      m_inv = inv;
    }

    /**
     * Set value for wire instance
     *
     * @param val
     */
    void set_val(int val) {
      m_wire->m_val = val;
    }

    /**
     * Get value of the wire instance
     *
     *
     * @return
     */
    int get_val() {
      return get_inv() ? m_wire->m_val ^ 1 : m_wire->m_val;
    }
  };

  /**
   * Gate
   *
   */
  class Gate {
  public:

    u32 m_func;

    WireInstance* m_in0, *m_in1, *m_out;

    u32 get_id() { return m_out->get_id(); }

    Gate() {}

    Gate(int func, WireInstance* in0, WireInstance* in1, WireInstance* out)
      : m_func(func), m_in0(in0), m_in1(in1), m_out(out) {}
  };
}

#endif
