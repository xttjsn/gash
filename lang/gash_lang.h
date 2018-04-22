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

  /**
   * Language level classes declarations
   */
  class Ast;
  class If;
  class For;
  class Ref;
  class Asgn;
  class Ret;           // Language return
  class Vardef;
  class Func;          // Function
  class Dir;           // Directive
  class Num;

  /**
   * A enumeration type for Abstract Syntax Tree
   * Each type correspond to a class
   */
  typedef enum {
    /* Arithmetic operations */
    aARITH,

    /* Comparison operations */
    aCMP,

    /* Bitwise operations */
    aAOXI,   // AND, OR, XOR, or Invert
    aSH,     // Left shift or right shift

    /* Language structure types */
    aNAME,  // Name
    aREF,   // Reference
    aASGN,  // Assign
    aNUM,   // Number
    aRET,   // Return
    aVARD,  // Variable definition

    /* Flow structure */
    aIF,
    aFOR,

    /* Directives */
    aDIR,   // Directive
  } NodeType;

  /**
   * A enumeration type for reference
   */
  typedef enum {
    rINT,
    rBIT,
  } RefType;

  typedef enum {
    dIP,
    dIN,
    dPORT,
    dROLE
  } DirType;

  /**
   * Abstract syntax tree definition
   * The trick is that every language structure has m_nodetype as its first
   * member, therefore we can safely cast an Ast to the appropriate class
   */
  class Ast {
  public:
    NodeType m_nodetype;
    Ast *m_l;
    Ast *m_r;
  };

  /**
   * If flow class
   *
   */
  class If {
  public:
    NodeType m_nodetype;
    Ast* m_cond_ast;  // The ast for the condition
    Ast* m_then_ast;  // The ast of code that's executed if condition is true
    Ast* m_else_ast;  // The ast of code that's executed if condition is false
    Scope* m_then_scope;
    Scope* m_else_scope;
  };

  /**
   * For flow class
   * Note that `init`, `inc`, `cond` is not executed using circuit evaluation but using normal evaluation
   */
  class For {
    NodeType m_nodetype;
    Ast* m_init_ast;  // The ast for initialization, it only gets to run once
    Ast* m_inc_ast;   // The ast for increment, it gets to run if cond is true
    Ast* m_cond_ast;  // The ast for condition, it gets to run once `do` finishes
    Ast* m_do_ast;    // The ast for doing the actual job, it's evaluated using circuit evaluation
  };

  /**
   * Num class
   *
   */
  class Num {
  public:
    NodeType m_nodetype;
    i64 m_val;
  };

  /**
   * Reference parent class
   *
   */
  class Ref {
  public:
    NodeType m_nodetype;
    RefType m_reftype;
    Symbol* m_sym;
  };

  /**
   * Reference to an integer
   *
   */
  class IntRef : public Ref {
  };

  /**
   * Reference to a bit
   *
   */
  class BitRef : public Ref {
  public:
    u32 m_bit_idx;
  };

  /**
   * An assignment operation
   *
   */
  class Asgn {
  public:
    NodeType m_nodetype;
    Ast* m_left;          // m_left can be a ref or a vardef
    Ast* m_right;         // m_right can be a number, or a ref
  };

  /**
   * Language return
   *
   */
  class Ret {
  public:
    NodeType m_nodetype;
    Ast* m_ret;          // m_ret can be a number, a ref or some expression
  };

  /**
   * Variable definition (single i64 value only)
   *
   */
  class Vardef {
  public:
    NodeType m_nodetype;
    u32 m_bitlen;
    i64 m_val;
  };

  /**
   * Function definition
   *
   */
  class Func {
  public:
    NodeType m_nodetype;
    char* m_funcname;
    Ast* m_vardef_list;
    Ast* m_do;
  };

  /**
   * Directive
   *
   */
  class Dir {
  public:
    NodeType m_nodetype;
    DirType m_dirtype;

    /// The symbol that this directive is going to provide input. Could be NULL
    /// if the directive does not use it
    Symbol* m_sym;

    /// The value that this directive provides, used with m_sym
    i64 m_val;

    //// Ip for dIP
    char* m_ip;

    //// Port for dPORT
    char* m_port;

    //// Role
    int m_role;
  };

  /**
   * The execution context used for invoking the gc framework.
   *
   */
  class ExeCtx {
  public:
    int m_role;
    u32 m_port;
    char* m_ip;
    char* m_circ_path;
    char* m_data_path;
  };

  /**
   * Create a new Ast from left Ast and right Ast
   *
   * @param nodetype
   * @param left
   * @param right
   *
   * @return
   */
  Ast* new_ast(NodeType nodetype, Ast* left, Ast* right);

  /**
   * Create a new arithmetic operation
   *
   * @param aop
   * @param left
   * @param right
   *
   * @return
   */
  Ast* new_aop(int aop, Ast* left, Ast* right);

  /**
   * Create a new bitwise operation
   *
   * @param bop
   * @param left
   * @param right
   *
   * @return
   */
  Ast* new_bop(int bop, Ast* left, Ast* right);

  /**
   * Create a new cmp Ast
   *
   * @param cmp_op
   * @param left
   * @param right
   *
   * @return
   */
  Ast* new_cop(int cop, Ast* left, Ast* right);

  /**
   * Create new function
   *
   * @param name
   * @param vardef_list
   * @param todo
   *
   * @return
   */
  Ast* new_func(char* name, Ast* vardef_list, Ast* todo);

  /**
   * Create new integer reference
   *
   * @param sym
   *
   * @return
   */
  Ast* new_ref_int(Symbol* sym);

  /**
   * Create new bit reference
   *
   * @param sym
   * @param bit_idx
   *
   * @return
   */
  Ast* new_ref_bit(Symbol* sym, u32 bit_idx);

  /**
   * Create new assignment
   *
   * @param left
   * @param right
   *
   * @return
   */
  Ast* new_asgn(Ast* left, Ast* right);

  /**
   * New number
   *
   * @param val
   *
   * @return
   */
  Ast* new_num(i64 val);

  /**
   * Create an if flow
   *
   * @param cond
   * @param then
   * @param else
   * @param then_scope
   * @param else_scope
   * @param prev_scope
   *
   * @return
   */
  Ast* new_if(Ast* cond, Ast* then, Ast* else, Scope* then_scope, Scope* else_scope, Scope* prev_scope);

  /**
   * Create a for loop
   *
   * @param init
   * @param cond
   * @param inc
   * @param todo
   * @param for_scope
   * @param prev_scope
   *
   * @return
   */
  Ast* new_for(Ast* init, Ast* cond, Ast* inc, Ast* todo, Scope* for_scope, Scope* prev_scope);

  /**
   * Create a return statement
   *
   * @param ret
   *
   * @return
   */
  Ast* new_ret(Ast* ret);

  /**
   * Create a new statement for variable definition
   *
   * @param sym
   * @param intlen
   * @param val
   *
   * @return
   */
  Ast* new_vardef(Symbol* sym, u32 intlen, i64 val);

  /**
   * Directive on input
   *
   * @param sym
   * @param val
   */
  void dir_input(Symbol* sym, i64 val);

  /**
   * Directive on ip
   *
   * @param x
   * @param y
   * @param w
   * @param z
   */
  void dir_ip(u8 x, u8 y, u8 w, u8 z);

  /**
   * Directive on port
   *
   * @param port
   */
  void dir_port(u32 port);

  /**
   * The core function of circuit building.
   * It takes a statement and evaluate it one after another (from right to left)
   * recursively.
   *
   * @param stat The statement that we want to evaluate
   * @param bundle The return bundle, allocate it on the stack please
   */
  void evalc(Ast* stat, Bundle& bundle);

  /**
   * This is another evaluation function. However, this function has no side
   * effect on circuit building, its only (now) purpose is to serve for loop's
   * init, condition, and increment
   *
   * @param stat The statement we want to evaluate
   *
   * @return
   */
  double evaln(Ast* stat);

  /**
   * Execute the circuit using Yao's protocol
   *
   * @param exectx The execution context
   */
  void exec(ExeCtx& exectx);
}
#endif
