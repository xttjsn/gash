/*
 * gash_lang.h -- Header file of GCDF compiler for gash
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

#ifndef GASH_LANG_H
#define GASH_LANG_H

#include "common.h"
#include "circuit.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

/* External: interface to the lexer */
extern int yylineno;    /* from lexer */
extern "C" void yyerror(const char *s, ...);
extern "C" int yyparse(void);

/* External: interface to the parser */
struct YYLTYPE;
extern "C" YYLTYPE yylloc;

/* External: counting */
extern int wcount;
extern int numIN;
extern int numOUT;
extern int numAND;
extern int numOR;
extern int numXOR;
extern int numDFF;

namespace gashlang {

  /* Enums */
  typedef enum {
    nPLUS = 1,
    nMINUS = 2,
    nUMINUS = 3,
    nNAME = 4,
    nREF = 5,
    nFUNC = 6,
    nMUL = 7,
    nDIV = 8,
    nIF = 9,
    nLIST = 10,
    nASGN = 11,
    nNUMBER = 12,
    nBUNDLENUMBER = 13,
    nNUMBERLIST = 14,
    nRETURN = 15,
    nVD = 16,          // Vardef: node for defining new variable
    nVDi = 17,         // Vardef inner variable
    nFOR = 18,         // For loop
    nVL = 19,          // Typedef list
    nLA = 20,          // >, larger than
    nLE = 21,          // <, less than
    nLAEQ = 22,        // >=, larger or equal
    nLEEQ = 23,        // <=, less or equal
    nEQ = 24,          // == equal
    nNEQ = 25,         // != not equal
    nBOR = 26,         // Bit-wise or
    nBAND = 27,        // Bit-wise and
    nBXOR = 28,        // Bit-wise xor
    nBNEG = 29,        // Bit-wise neg
    nDIR = 30,         // Directive
    nDIRLIST = 31,     // List of directives
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

  /**
   * Language level classes
   */
  class scope;
  class symbol;
  class ast;
  class flow;
  class symref;
  class symasgn;
  class l_return;   // Language return
  class vardef;
  class vardef_list;
  class dir;
  class num_list;

  /**
   * Function: symbol lookup
   */
  symbol* lookup(char* name);
  symbol* lookup(string name);

  /**
   * Look up symbol with name 'name' of version 'version' in scope store (i.e. global scope).
   *
   * @param name
   * @param version
   *
   * @return
   */
  symbol* lookup(string name, u32 version);

  /**
   * Look up symbol with 'name' of version 'version' in scope 'scope'.
   *
   * @param name
   * @param version
   * @param scope
   *
   * @return
   */
  symbol* lookup(string name, u32 version, scope* scope);



}

#endif
