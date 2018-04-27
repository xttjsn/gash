/*
 * arith.h -- Contain macros of arithmetic & bitwise operations
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

#ifndef GASH_OP_H
#define GASH_OP_H

#include "circuit.h"

namespace gashlang {

  /* Arithmetic operations */
#define AOP_PLUS   0x00
#define AOP_SUB  0x01
#define AOP_UMINUS 0x02
#define AOP_MUL    0x03
#define AOP_DIV    0x04

  /// If advanced arithmetic is enabled
#ifdef __ADV_ARITH__

#define AOP_SQR   0x05
#define AOP_SQRT  0x06

  /// If piece wise linear approximation is enabled
#ifdef __PWS_LIN_APPRX__

#define AOP_LOG2  0x07
#define AOP_LOG10 0x08

#endif
#endif

  /* Bitwise operations */
#define BOP_OR     0x10
#define BOP_AND    0x11
#define BOP_XOR    0x12
#define BOP_INV    0x13
#define BOP_SHL    0x14
#define BOP_SHR    0x15

  /* Comparison operations */
#define COP_LA     0x20  // Larger than
#define COP_LE     0x21  // Less than
#define COP_LAE    0x22  // Larger than or equal to
#define COP_LEE    0x23  // Less or equal to
#define COP_EQ     0x24  // Equal
#define COP_NEQ    0x25  // Not equal


  /* Gate type */
#define AND 8
#define OR 14
#define XOR 6

  /* Bundle-level evaluation function */
  /**
   * Evaluate a full adder
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evala_ADD(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Evaluate a full subtractor
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evala_SUB(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Evaluate multiplication
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evala_MUL(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Evaluate division
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evala_DIV(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Support function for division
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evala_DVG(Bundle& in0, Bundle& in1, Bundle& out, Wire*& ret);

  /**
   * Uminus
   *
   * @param in
   * @param out
   *
   * @return
   */
  int evala_UMINUS(Bundle& in, Bundle& out);

  /**
   * Evaluate ANDs
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evalb_AND(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Evaluate ORs
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evalb_OR(Bundle& in0, Bundle& in1, Bundle& out);


  /**
   * Evaluate XORs
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evalb_XOR(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Invert bundle
   *
   * @param in
   * @param out
   *
   * @return
   */
  int evalb_INV(Bundle& in, Bundle& out);

  /**
   * Evaluate bitwise left shift
   *
   * @param in0
   * @param n
   *
   * @return
   */
  int evalb_SHL(Bundle& in0, u32 n, Bundle& out);

  /**
   * Evaluate bitwise right shift
   *
   * @param in0
   * @param n
   *
   * @return
   */
  int evalb_SHR(Bundle& in0, u32 n, Bundle& out);

  /**
   * CMP: Larger than
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evalc_LA(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * CMP: Less than
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evalc_LE(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Equal
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evalc_EQ(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Less than or equal to
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evalc_LEE(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Large than or equal to
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evalc_LAE(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Not equal
   *
   * @param in0
   * @param in1
   * @param out
   *
   * @return
   */
  int evalc_NEQ(Bundle& in0, Bundle& in1, Bundle& out);

  /**
   * Assign then_res/else_res to out conditioned on cond
   *
   * @param cond
   * @param then_res
   * @param else_res
   * @param out
   *
   * @return
   */
  int evalo_if(Wire* cond, Bundle& then_res, Bundle& else_res, Bundle& out);

  /**
   * Write an AND/OR/XOR gate to circuit m_gates
   *
   * @param op
   * @param in0
   * @param in1
   * @param out
   */
  void write_gate(int op, Wire* in0, Wire* in1, Wire*& out);

  /* Wire-level evaluation function */

  /**
   * Evaluate a full adder
   *
   * @param in0
   * @param in1
   * @param cin
   *
   * @return
   */
  int evalw_FADD(Wire* in0, Wire* in1, Wire*& cin, Wire*& ret);

  /**
   * Evaluate a full subtractor
   *
   * @param in0
   * @param in1
   * @param prev_bout
   * @param bout
   *
   * @return
   */
  int evalw_FSUB(Wire* in0, Wire* in1, Wire*& bout, Wire*& ret);

  /**
   * Evaluate bitwise AND
   *
   * @param in0
   * @param in1
   *
   * @return
   */
  int evalw_AND(Wire* in0, Wire* in1, Wire*& ret);

  /**
   * Evaluate bitwise OR
   *
   * @param in0
   * @param in1
   *
   * @return
   */
  int evalw_OR(Wire* in0, Wire* in1, Wire*& ret);

  /**
   * Evaluate bitwise XOR
   *
   *
   * @return
   */
  int evalw_XOR(Wire* in0, Wire* in1, Wire*& ret);

  /**
   * Evaluate bitwise invert
   *
   * @param in0
   *
   * @return
   */
  int evalw_INV(Wire* in0, Wire*& ret);

}

#endif
