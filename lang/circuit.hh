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

#ifndef GASH_LANG_CIRCUIT_H
#define GASH_LANG_CIRCUIT_H

#include "../include/common.hh"
#include "op.hh"

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
  typedef map<u32, u32> IdValMap;
  typedef map<u32, u32> IdIdMap;
  typedef map<Wire*, Wire*> WireWireMap;
  typedef map<u32, Wire*> IdWireMap;
  typedef vector<u32> Bits;

  /**
   * The wire class
   *
   */
  class Wire {
  public:
    u32 m_id = 0;

    u32 m_refcount = 0;
    //// v should be set only when its a constant wire
    i32 m_v = -1;
    Gate* m_parent_gate = NULL;
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
     * Make this wire an exact duplicate of another wire, except that the oddness of id are always inverted.
     *
     * @param w
     */
    void invert_from(Wire* w);

    /**
     * Copy from another wire
     *
     * @param w
     */
    void copyfrom(Wire* w);

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

  Wire* onewire();

  Wire* zerowire();

  Wire* nextwire();

  class Bundle {
  public:
    vector<Wire*> m_wires;
    map<u32, Wire*> m_wires_map;

    /// true if this bundle is derived from a constant value.
    /// Default false.
    /// If true, then during assignment, it is allowed to use only part
    /// of the bundle
    bool          m_isconst;

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
     * Add wire into the bundle (at the end)
     *
     * @param w
     */
    void add(Wire* w);

    /**
     * Add wire to position `i`, `i` must be less than or equal to size()
     *
     * @param w
     * @param i
     */
    void add(Wire* w, u32 i);

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
       * Insert wire at the front
       *
       * @param w
       */
      void push_front(Wire* w);

    /**
     * Return the last wire
     *
     *
     * @return
     */
    Wire* back();

    /**
     * Remove wire at index `i`
     *
     * @param i
     *
     * @return
     */
    int remove(u32 i);

    /**
     * Get the wire with idx `i`
     *
     * @param i
     *
     * @return
     */
    Wire* getWire(u32 i);

      /**
       * Set wire
       *
       * @param w
       * @param i
       */
      int setWire(Wire* w, u32 i);

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
     * Remove all wire from bundle, decrease refcount of each wire by 1
     *
     */
    void clear();

    /**
     * Set the value of all wires in this bundle to 0.
     *
     */
    void clear_val();

    /**
     * Emit one wire per line
     *
     * @param outsteam
     */
    void emit(ostream& outsteam);

  };

  /**
   * Convert integer to bundle, with specified size n
   *
   * @param v
   * @param bundle
   * @param n
   */
  void num2bundle_n(i64 v, Bundle& bundle, u32 n);

  /**
   * Convert 64-bit value to bundle
   *
   * @param v
   * @param bundle
   */
  void num2bundle(i64 v, Bundle& bundle);

  /**
   * Prologue is responsible for writing the first line of the output circuit.
   *
   */
  class Prologue {
  public:
    u32 numVAR;
    u32 numIN ;
    u32 numOUT;
    u32 numAND;
    u32 numOR ;
    u32 numXOR;
    u32 numDFF;

    Prologue() {
    }

    Prologue(u32 IN, u32 OUT, u32 AND, u32 OR, u32 XOR, u32 DFF);

    void emit(ostream& outstream);
  };

  class Gate {
  public:
    // AND/OR/XOR
    int m_op;
    /// First input wire
    Wire* m_in0;
    /// Second input wire
    Wire* m_in1;
    /// Output wire
    Wire* m_out;

    Gate() {}

    Gate(int op, Wire* in0, Wire* in1, Wire* out)
        : m_out(out)
        , m_op(op)
        , m_in0(in0)
        , m_in1(in1)
    {
      out->m_parent_gate = this;
    }

    void emit(ostream &outstream);
  };

  class GateList {
  public:
    vector<Gate*>       m_gates;
    void emit(ostream& outstream);
    void add(Gate* g) {m_gates.push_back(g);}
  };

  class Circuit {
  public:
    Prologue      m_prologue;
    Bundle        m_in;
    Bundle        m_out;

    GateList      m_gates;
    IdWireMap     m_wires;
    IdIdMap       m_input_dup;
    IdIdMap       m_wire_inverts;
    ostream*      m_circ_stream = NULL;
    ostream*      m_data_stream = NULL;

    Circuit() {
      m_prologue = Prologue();
      m_prologue.numAND = 0;
      m_prologue.numVAR = 0;
      m_prologue.numIN  = 0;
      m_prologue.numOUT = 0;
      m_prologue.numAND = 0;
      m_prologue.numOR  = 0;
      m_prologue.numXOR = 0;
      m_prologue.numDFF = 0;
    }
    Circuit(ostream& circ_stream, ostream& data_stream);

    /**
     * Write the entire circuit to circ_stream and data_stream.
     *
     */
    void write();

    /**
     * Write input wire to circuit file
     *
     */
    void write_inwires();

    /**
     * Write output wire to circuit file
     *
     */
    void write_outwires();

    /**
     * Write input to data_stream.
     *
     */
    void write_input();

    /**
     * Add wire to circuit
     *
     * @param w
     */
    void add_wire(Wire* w);

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
    bool has_input_dup(Wire* w);

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
    void set_input_inv_dup(Wire* w, Wire* w_dup);

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

    /**
     * Add a gate to circuit
     *
     * @param op
     * @param in0
     * @param in1
     * @param out
     */
    void add_gate(int op, Wire* in0, Wire* in1, Wire* out);
  };

    void print_idw_map(map<u32, Wire*> wires_map);
}  // gashlang

#endif
