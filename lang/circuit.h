/*
 * circuit.h -- Circuit related classes
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

#ifndef GASH_CIRCUIT_H
#define GASH_CIRCUIT_H

#include "common.h"

namespace gashlang {

  /**
   * Circuit level classes
   */
  class Wire;
  class Bundle;
  class Prologue;
  class Gate;
  class GateList;
  class Circuit;

  /**
   * Typedef a bunch of helper classes
   *
   */
  typedef map<u32, u32> WireIdValueMap;
  typedef map<u32, u32> WireIdWireIdMap;
  typedef map<Wire*, Wire*> WireWireMap;
  typedef map<u32, Wire*> WireIdWireMap;
  typedef vector<u32> Bits;

  /**
   * The wire class
   *
   */
  class Wire {
  public:
    u32 m_id = 0;

    //// v should be set only when its a constant wire
    i32 m_v = -1;
    Gate* m_parent_gate;
    vector<Gate*> m_children_gates;

    /// Used for inverting duplicated wire
    bool m_used_once = false;

    Wire() {}
    Wire(u32 id) : m_id(id) {}
    Wire(u32 id, i32 v) : m_id(id), m_v(v) {}

    /**
     * Invert this wire.
     */
    void invert();

    void set_id_even();

    void set_id_odd();

    /**
     * Make this wire an exact duplicate of another wire, except that the values are always inverted.
     *
     * @param w
     */
    void invert_from(Wire* w);

    /**
     * Create indication that this wire is used.
     */
    inline void used() {
      m_used_once = true;
    }

    /**
     * Notify caller whether this wire has been used or not.
     * @return boolean
     */
    inline bool is_used_once() {
      return m_used_once;
    }
  };

  class Bundle {
  public:
    vector<Wire*> m_wires;
    map<u32, Wire*> m_wires_map;

    Bundle() {}

    /**
     * Create a bundle with a vector of already-created wires.
     *
     * @param wires
     */
    Bundle(vector<Wire*>& wires);

    /**
     * Create a bundle with a single wire.
     *
     * @param w
     */
    Bundle(Wire* w);

    /**
     * Create a bundle with 'len'-many newly created wires.
     *
     * @param len
     */
    Bundle(u32 len);

    /**
     * Add wire into the bundle
     *
     * @param w
     */
    void add(Wire* w);

    /**
     * Check whether it has a specific wire
     *
     * @param i
     *
     * @return
     */
    bool hasWire(u32 i);

    /**
     * Access the 'i'-th wire in the bundle
     *
     * @param i
     *
     * @return
     */
    Wire* operator[](u32 i);

    /**
     * Get the wire with idx `i`
     *
     * @param i
     *
     * @return
     */
    Wire* getWire(u32 i);

    /**
     * Size of the bundle.
     *
     * @return
     */
    u32 size();

    /**
     * Copy the 'src_start'-th wire till 'src_start + size'-th wire from src to this bundle
     *
     * @param r
     * @param start
     * @param rstart
     * @param size
     *
     * @return 0 if success, otherwise return 1.
     */
    int copyfrom(Bundle& src, u32 start, u32 src_start, u32 size);

    /**
     * Set the value of all wires in this bundle to 0.
     *
     */
    void clear_val();

    /**
     * Write the wire to output stream.
     *
     * @param outstream
     */
    void emit(ostream& outstream);
  };

  /**
   * Prologue is responsible for writing the first line of the output circuit.
   *
   */
  class Prologue {
  public:
    u32 numIN;
    u32 numOUT;
    u32 numAND;
    u32 numOR;
    u32 numXOR;
    u32 numDFF;

    Prologue() {}

    Prologue(u32 IN, u32 OUT, u32 AND, u32 OR, u32 XOR, u32 DFF);

    void emit(ostream& outstream);
  };

  class Gate {
  public:
    /// Output wire
    Wire* m_out;
    Op m_op;
    /// First input wire
    Wire* m_in0;
    /// Second input wire
    Wire* m_in1;

    Gate() {}

    Gate(Wire* out, Op op, Wire* in0, Wire* in1);

    void emit(ostream &outstream);
  };

  class GateList {
  public:
    vector<Gate*> m_gates;
    void emit(ostream& outstream);
  };

  class Circuit {
  public:
    Prologue m_prologue;
    Bundle m_in;
    Bundle m_out;

    GateList m_gates;
    WireIdWireMap m_wires;
    WireIdValueMap m_input_data;
    WireIdWireIdMap m_input_duplicates;
    WireIdWireIdMap m_wire_inverts;
    ostream* m_circ_stream;
    ostream* m_data_stream;

    Circuit(ostream& circ_stream, ostream& data_stream);

    /**
     * Write the entire circuit to circ_stream and data_stream.
     *
     */
    void write();


    /**
     * Write input to data_stream.
     *
     */
    void write_input();

    /**
     * Use the bundle as input, will take care of input duplicates.
     * Additionally, we check the consistency against m_in.
     * As a result, call this function only after m_in is properly setup.
     *
     * @param bits
     * @param b
     */
    void add_input_values(Bundle& bundle);

    /**
     * Add input wire to the circuit.
     *
     * @param w
     */
    void add_input_wire(Wire* w);

    /**
     * Check whether wire w has duplicates.
     *
     * @param w
     *
     * @return
     */
    bool has_input_duplicate(Wire* w);

    /**
     * Check whether the wire is input wire.
     *
     * @param w
     *
     * @return
     */
    bool is_input_wire(Wire* w);

    /**
     * Set w_dup to be the duplicate of w, as w to be the duplicate of w_dup.
     *
     * @param w
     * @param w_dup
     */
    void set_input_invert_duplicate(Wire* w, Wire* w_dup);

    /**
     * Check whether wire has inverted wire,
     *
     * @param w
     *
     * @return
     */
    bool has_invert_wire(Wire* w);

    /**
     * Get the inverted wire, will return NULL is nothing found.
     *
     * @param w
     *
     * @return
     */
    Wire* get_invert_wire(Wire* w);

    /**
     * Set w_inv to be the inverted wire of w, and w to be the inverted wire of w_inv.
     *
     * @param w
     * @param w_inv
     *
     * @return
     */
    int set_invert_wire(Wire* w, Wire* w_inv);

    /**
     * Set the circuit's output stream
     *
     * @param outstream
     */
    inline void set_circ_outstream(ostream& outstream) {
      m_circ_stream = &outstream;
    }

    /**
     * Set the circuit's data output stream
     *
     * @param outstream
     */
    inline void set_data_outstream(ostream& outstream) {
      m_data_stream = &outstream;
    }
  };

}  // gashlang

#endif
