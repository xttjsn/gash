/*
 * garbled_circuit.h -- Garbled circuit header
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

#ifndef GASH_GARBLED_CIRCUIT_H
#define GASH_GARBLED_CIRCUIT_H

#include <wmmintrin.h>

#include "../include/common.hh"
#include "circuit.hh"

#define WI     WireInstance
#define GWI    GarbledWireInstance
#define GC     GarbledCircuit
#define GG     GarbledGate

namespace gashgc {

  class GarbledGate;
  class GarbledWireInstance;
  class GarbledCircuit;
  class LabelPair;
  class OutputMap;

  /**
   * Typedef a few commonly used classes for simplicity
   *
   */
  typedef map<u32, GarbledWireInstance*> IdGarbledWireinsMap;
  typedef map<u32, GarbledGate*> IdGarbledGateMap;
  typedef map<u32, LabelPair*> IdLabelPairMap;
  typedef map<u32, block> IdLabelMap;
  typedef map<u32, u32>   IdValueMap;
  typedef vector<u32>     IdVec;
  typedef set<u32>        IdSet;


  /**
   * Garbled Circuit
   *
   */
  class GarbledCircuit {
  public:
    u32                        m_nin;
    u32                        m_nout;
    u32                        m_ngate;
    block                      m_R;

    IdGarbledWireinsMap        m_gwi_map;
    IdGarbledGateMap           m_gg_map;

    /**
     * Get Garbled Wire Instance from id
     *
     * @param id
     *
     * @return
     */
    GWI* get_gwi(u32 id);

    /**
     * Get garbled gate from id
     *
     * @param id
     *
     * @return A pointer to the garbled gate, NULL if not find
     */
    GG* get_gg(u32 id);

    /**
     * Add garbled gate
     *
     * @param circ
     * @param id
     * @param row1
     * @param row2
     * @param row3
     *
     * @return
     */
    int add_gg(Circuit &circ, u32 id, block row1, block row2, block row3);

    /**
     * Add garbled gate
     *
     * @param gg
     *
     * @return
     */
    int add_gg(GG* gg);

    /**
     * Return 1 if has garbled gate, otherwise return 0
     *
     * @param id
     *
     * @return
     */
    bool has_gg(u32 id);

    /**
     * Add garbled wire instance
     *
     * @param gwi
     *
     * @return
     */
    int add_gwi(GWI* gwi);

    /**
     * Return 1 if has gwi with id `id`, otherwise return 0
     *
     * @param id
     *
     * @return
     */
    bool has_gwi(u32 id);

    /**
     * Set garbled wire label
     *
     * @param id
     * @param label
     *
     * @return
     */
    int set_gwl(u32 id, block label);

    /**
     * Get label for wire with id `id` for semantic `val`
     *
     * @param id
     * @param val
     * @param lbl
     *
     * @return
     */
    int get_lbl(u32 id, int val, block& lbl);

    /**
     * Init R
     *
     */
    void init();

    /**
     * Print debug report as evaluator
     *
     */
    void debug_report_evaluator();

    /**
     * Print debug report as garbler
     *
     */
    void debug_report_garbler();

    /**
     * Constructor
     *
     */
    GarbledCircuit();

    /**
     * Destructor
     *
     */
    ~GarbledCircuit();

  };


  /**
   * Garbled Wire
   *
   */
  class GarbledWire {
  public:
    u32                    m_id;
    block                  m_lbl0;               // Used for garbling
    block                  m_lbl1;
    block                  m_lbl;                // Used for evaluating

    GarbledWire () {}

    /**
     * Constructor from wire instance
     *
     * @param w
     */
    GarbledWire (WireInstance* w) : m_id(w->get_id()) {}

    /**
     * Constructor from id
     *
     * @param id
     */
    GarbledWire (u32 id) : m_id(id) {}

    /**
     * Destructor
     *
     */
    ~GarbledWire() {}
  };

  /**
   * Garbled Wire Instance
   *
   */
  class GarbledWireInstance {
  public:
    GarbledWire*                       m_gw = NULL;
    bool                               m_inv = false;

    /**
     * Default constructor
     *
     */
    GarbledWireInstance() {}

    // GarbledWireInstance(u32 id) : garbled_wire_ptr_(new GarbledWire(id)), inverted_(false) {}

    /**
     * Constructor from wire instance
     *
     * @param w
     */
    GarbledWireInstance(WireInstance* w) : m_gw(new GarbledWire(w)), m_inv(w->get_inv()) {}

    /**
     * Get label 0
     *
     * @return
     */
    block get_lbl0() {
      return m_inv ? m_gw->m_lbl1: m_gw->m_lbl0;
    }

    /**
     * Get label 1
     *
     * @return
     */
    block get_lbl1() {
      return m_inv ? m_gw->m_lbl0: m_gw->m_lbl1;
    }

    /**
     * Get label
     *
     * @return
     */
    block get_lbl() {
      return m_gw->m_lbl;
    }

    /**
     * Set label 0
     *
     * @param lbl0
     */
    void set_lbl0(block lbl0) {
      if (m_inv) {
        m_gw->m_lbl1 = lbl0;
      } else {
        m_gw->m_lbl0 = lbl0;
      }
    }

    /**
     * Set label 1
     *
     * @param lbl1
     */
    void set_lbl1(block lbl1) {
      if (m_inv) {
        m_gw->m_lbl0 = lbl1;
      } else {
        m_gw->m_lbl1 = lbl1;
      }
    }

    /**
     * Set label
     *
     * @param lbl
     */
    void set_lbl(block lbl) {
      m_gw->m_lbl = lbl;
    }

    /**
     * Set id
     *
     * @param id
     */
    void set_id(u32 id) {
      m_gw->m_id = id;
    }

    /**
     * Get id
     *
     * @return
     */
    u32 get_id() {
      return m_gw->m_id;
    }

    /**
     * Garble this wire
     *
     * @param R
     */
    void garble(block R);

    /**
     * Return 1 if inverted, otherwise return 0
     *
     * @return
     */
    bool get_inv() {
      return m_inv;
    }

    /**
     * Get the semantic (true value) with label that has the least significant bit
     *
     * @param lsb
     *
     * @return
     */
    int get_smtc_w_lsb(int lsb);

    /**
     * Get the label that has the semantic
     *
     * @param semantic
     *
     * @return
     */
    block get_lbl_w_smtc(int semantic);

    /**
     * Get the original label that has the semantic
     *
     * @param semantic
     *
     * @return
     */
    block get_orig_lbl_w_smtc(int semantic);

    /**
     * Get the label that has the least significant bit
     *
     * @param lsb
     *
     * @return
     */
    block get_lbl_w_lsb(int lsb);

    /**
     * Set label that has the semantic
     *
     * @param lbl
     * @param semantic
     *
     * @return 0 if success, errno if failure
     */
    int set_lbl_w_smtc(block lbl, int semantic);

    /**
     * Recover the semantic based on lbl0, lbl1, and lbl
     *
     *
     * @return 0/1 if success, negative errno if failed
     */
    int recover_smtc();

    /**
     * Destructor
     *
     */
    ~GarbledWireInstance() {
      delete m_gw;
    }
  };


  /**
   * Encrypted Garbled Truth Table
   *
   */
  class EGTT {
  public:

#ifndef GASH_GC_GRR
    block                   m_row0;
#endif

    block                   m_row1;
    block                   m_row2;
    block                   m_row3;

    EGTT() {}

#ifdef GASH_GC_GRR

    EGTT(block row1, block row2, block row3) {
      m_row1 = row1;
      m_row2 = row2;
      m_row3 = row3;
    }

#else

    EGTT(block row0, block row1, block row2, block row3) {
      m_row0 = row0;
      m_row1 = row1;
      m_row2 = row2;
      m_row3 = row3;
    }

#endif

    block get_row(int i) {

        block b;

      switch (i) {

#ifdef GASH_GC_GRR
      case 0:
        b = getZEROblock();
	break;
#else
      case 0:
        b = m_row0;
	break;
#endif

      case 1:
        b = m_row1;
	break;

      case 2:
        b = m_row2;
	break;

      case 3:
        b = m_row3;
	break;

      default:
        FATAL("Invalid row selection: " << i << "\n");
      }

      return b;
    }

    /**
     * Set row 1
     *
     * @param row1
     */
    void set_row1(block row1) { m_row1 = row1; }

    /**
     * Set row 2
     *
     * @param row2
     */
    void set_row2(block row2) { m_row2 = row2; }

    /**
     * Set row 3
     *
     * @param row3
     */
    void set_row3(block row3) { m_row3 = row3; }

    /**
     * Destructor
     *
     */
    ~EGTT() {}
  };


  /**
   * Garbled Gate
   *
   */
  class GarbledGate {
  public:
    bool m_is_xor;
    GWI *m_in0;
    GWI *m_in1;
    GWI *m_out;
    EGTT *m_egtt;

    GarbledGate() {}

    /**
     * Constructor with only wires
     *
     * @param id
     * @param is_xor
     * @param in0
     * @param in1
     * @param out
     */
    GarbledGate(bool is_xor, GWI* in0, GWI* in1, GWI* out):
      m_is_xor(is_xor),  m_in0(in0),  m_in1(in1),  m_out(out) {
    }

    /**
     * Constructor with wires and egtt
     *
     * @param is_xor
     * @param in0
     * @param in1
     * @param out
     * @param egtt
     */
    GarbledGate(bool is_xor, GWI* in0, GWI* in1, GWI* out, EGTT* egtt): m_is_xor(is_xor), m_in0(in0), m_in1(in1), m_out(out), m_egtt(egtt) {
    }

    /**
     * Get the id for the garbled gate
     *
     *
     * @return
     */
    inline u32 get_id() {
      if (!m_out)
        FATAL("Garbled gate does not have output wire");
      return m_out->get_id();
    }

    /**
     * Destructor
     *
     */
    ~GarbledGate() {
      delete m_in0;
      delete m_in1;
      delete m_out;
      delete m_egtt;
    }
  };

  /**
   * A pair of labels with semantic as the index
   *
   */
  class LabelPair {
  public:
    block m_lbl0;
    block m_lbl1;

    /**
     * Marshalling label pair to buffer `dest`
     *
     * @param dest
     * @param size The byte size of the buffer that will be marshaled to
     *
     * @return The byte size that's marshaled
     */
    int marshal(char* dest, u32 size);

    /**
     * Unmarshal from buffer `src`
     *
     * @param src
     * @param size Byte size of the `src` buffer
     *
     * @return The byte size that's successfully unmarshaled
     */
    int unmarshal(char* src, u32 size);

    /**
     * Get the semantic of the label, return error if no matching found
     *
     * @param lbl
     *
     * @return 0 if stmc == 0, 1 if stmc == 1, negative if error
     */
    int get_smtc(block& lbl);

  };

  class GWLMap {
  public:
    IdLabelPairMap m_lbl_pair_map;

    /**
     * Add label pair
     *
     * @param id
     * @param lbl0
     * @param lbl1
     *
     * @return 0 if success, negative if error
     */
    int add_lbl_pair(u32 id, block lbl0, block lbl1);

    /**
     * Get the semantic for the wire with `id` corresponding to label `lbl`
     *
     * @param id
     * @param lbl
     *
     * @return The semantic for the label
     */
    int get_smtc(u32 id, block lbl);

    /**
     * Get label for wire with `id` and for semantic `smtc`
     *
     * @param id
     * @param smtc
     * @param lbl
     *
     * @return
     */
    int get_lbl(u32 id, int smtc, block& lbl);

    /**
     * Marshal the entire output map
     *
     * @param dest
     *
     * @return The byte size that's successfully marshaled
     */
    int marshal(char* dest, u32 size);

    /**
     * Unmarshal from buffer `src`
     *
     * @param src
     * @param size Byte size of the `src` buffer
     *
     * @return The byte size that's successfully unmarshaled
     */
    int unmarshal(char* src, u32 size);

    /**
     * The size that's required to receive the entirety of the marshaled output map
     *
     *
     * @return
     */
    u32 marshal_size();
  };

}

#endif
