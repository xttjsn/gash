/*
 * ast.h -- Abstract Syntax Tree implementation
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

#ifndef GASH_AST_H
#define GASH_AST_H

#include "common.h"
#include "sym.h"

namespace gashlang {

  /* Enums */
  typedef enum {

    /* Arithmetic nodes */
    nPLUS,
    nMINUS,
    nUMINUS,
    nMUL,
    nDIV,

    /* Boolean operation nodes */
    nLA,          // >, larger than
    nLE,          // <, less than
    nLAEQ,        // >=, larger or equal
    nLEEQ,        // <=, less or equal
    nEQ,          // == equal
    nNEQ,         // != not equal
    nBOR,         // Bit-wise or
    nBAND,        // Bit-wise and
    nBXOR,        // Bit-wise xor
    nBNEG,        // Bit-wise neg


    nNAME,
    nREF,

    nFUNC,

    nASGN,

    nLIST,

    nNUMBER,
    nBUNDLENUMBER,
    nNUMBERLIST,


    nVD,          // Vardef: node for defining new variable
    nVDi,         // Vardef inner variable

    nIF,           // If else
    nFLOW,        // Generic flow ast
    nFOR,         // For loop

    nRETURN,

    nVL,          // Typedef list
    nDIR,         // Directive
    nDIRLIST,     // List of directives
  } Knodetype;

  // Reference types other than symbol
  typedef enum {
    rBUNDLE,		                    // e.g. in1
    rBUNDLE_dot_WIRE,               // e.g. in1.(i+j)
    rARRAY_dot_BUNDLE,              // e.g. in1[i+j]
    rARRAY_dot_BUNDLE_dot_WIRE,     // e.g. in1[i+j].(k+l)
  } Kreftype;

  typedef enum {
    dSINGLE,
    dARRAY,
    dROLE,
    dPORT,
    dIP,
    dEXPOUT
  } Kdirtype;

  class Ast {
  public:
    int m_nodetype;
    Ast *m_l;
    Ast *m_r;
  };

  class Flow : public Ast {
  public:
    int m_nodetype =;
  };

  class FlowIfElse {
    Ast* m_init;        // for
    Ast* m_cond;        // for and if
    Ast* m_inc;         // for
    Ast* m_list;        // for
    Ast* m_tl;          // if
    Ast* m_el;          // if
    Scope* m_tl_scope;
    Scope* m_el_scope;
    Scope* m_for_scope;
    Scope* m_orig_scope;

  }

}  // gashlang

#endif
