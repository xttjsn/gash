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

namespace gashlang {

  /* Arithmetic operations */
#define AOP_PLUS   0x00
#define AOP_MINUS  0x01
#define AOP_UMINUS 0x02
#define AOP_MUL    0x03
#define AOP_DIV    0x04

  /// If advanced arithmetic is enabled
#ifdef __ADV_ARITH__

#define AOP_SQR   0x05
#define AOP_SQRT  0x06

  /// If piece wise linear approximation is enabled
#ifdef __PWS_LIN_APPROX__

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

}

#endif
