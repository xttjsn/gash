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

#include "../include/common.h"
#include "circuit.h"
#include <wmmintrin.h>

namespace gashgc {

  class GarbledGate;
  class GarbledWireInstance;
  class GarbledCircuit;

  /**
   * Typedef a few commonly used classes for simplicity
   *
   */
  typedef map<u32, GarbledWireInstance*> IdGarbledWireinsMap;
  typedef map<u32, GarbledGate*> IdGarbledGateMap;

#define GWI GarbledWireInstance
#define GC GarbledCircuit
#define GG GarbledGate

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
     * @return
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

    // GarbledWireInstance(GarbledWire & garbled_wire) : garbled_wire_ptr_(&garbled_wire), inverted_(false) {}

    // GarbledWireInstance(GarbledWire* garbled_wire_ptr) : garbled_wire_ptr_(garbled_wire_ptr), inverted_(false) {}

    // GarbledWireInstance(GarbledWire & garbled_wire, bool inverted) : garbled_wire_ptr_(&garbled_wire), inverted_(inverted) {}

    // GarbledWireInstance(GarbledWire* garbled_wire_ptr, bool inverted) : garbled_wire_ptr_(garbled_wire_ptr), inverted_(inverted) {}

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
    block lbl() {
      return m_gw->m_lbl;
    }

    /**
     * Set label 0
     *
     * @param lbl0
     */
    void set_lbl0(block lbl0) {
      m_gw->m_lbl0 = lbl0;
    }

    /**
     * Set label 1
     *
     * @param lbl1
     */
    void set_lbl1(block lbl1) {
      m_gw->m_lbl1 = lbl1;
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
    int get_lbl_w_smtc(int semantic);

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
     */
    void set_lbl_w_smtc(block lbl, int semantic);

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
      switch (i) {

#ifdef GASH_GC_GRR
      case 0:
        return getZEROblock();
#else
      case 0:
        return m_row0;
#endif

      case 1:
        return m_row1;

      case 2:
        return m_row2;

      case 3:
        return m_row3;

      default:
        FATAL("Invalid row selection: " << i << "\n");
      }
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

  };


  /**
   * Garbled Gate
   *
   */
  class GarbledGate {
  public:
    u32 m_id;
    bool m_is_xor;
    GWI *m_in0;
    GWI *m_in1;
    GWI *m_out;
    EGTT *m_egtt;

    GarbledGate() {}

    /**
     * Constructor
     *
     * @param id
     * @param is_xor
     * @param in0
     * @param in1
     * @param out
     */
    GarbledGate(bool is_xor, GWI* in0, GWI* in1, GWI* out):
      m_is_xor(is_xor),  m_in0(in0),  m_in1(in1),  m_out(out) {
      m_id = out->get_id();
    }

    u32 get_id() { return m_out->get_id(); }
  };

}

#endif
