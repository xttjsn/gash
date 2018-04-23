/*
 * gash_lang.cpp -- The GCDF compiler of gash
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

#include "gash_lang.h"

namespace gashlang {

  static Circuit mgc;

  Ast* new_ast(NodeType ntype, Ast* left, Ast* right) {
    Ast* ast = new Ast;
    REQUIRE_NOT_NULL(ast);
    ast->m_nodetype = ntype;
    ast->m_l = left;
    ast->m_r = right;
    return ast;
  }

  Ast* new_aop(int aop, Ast* left, Ast* right) {
    Aop* aop = new Aop;
    REQUIRE_NOT_NULL(aop);
    aop->m_nodetype = nAOP;
    aop->m_left = left;
    aop->m_right = right;
    return (Ast*) aop;
  }

  Ast* new_bop(int bop, Ast* left, Ast* right) {
    Bop* bop = new Bop;
    REQUIRE_NOT_NULL(bop);
    bop->m_nodetype = nBOP;
    bop->m_left = left;
    bop->m_right = right;
    return (Ast*) bop;
  }

  Ast* new_cop(int cop, Ast* left, Ast* right) {
    Cop* cop = new Cop;
    REQUIRE_NOT_NULL(cop);
    cop->m_nodetype = nCOP;
    cop->m_left = left;
    cop->m_right = right;
    return (Ast*) cop;
  }

  Ast* new_func(char* name, Ast* vardef_list, Ast* todo) {
    Func* func = new Func;
    func->m_funcname = name;
    func->m_vardef_list = vardef_list;
    func->m_do = todo;
    return (Ast*)func;
  }

  Ast* new_ref_int(Symbol* sym) {
    IntRef* ref = new IntRef;
    ref->m_reftype = rINT;
    ref->m_sym = sym;
    return (Ast*) ref;
  }

  Ast* new_ref_bit(Symbol* sym, u32 bit_idx) {
    BitRef* ref = new BitRef;
    ref->m_reftype = rBIT;
    ref->m_sym = sym;
    ref->m_bit_idx = bit_idx;
    return (Ast*) ref;
  }

  Ast* new_asgn(Ast* left, Ast* right) {
    Asgn* asgn = new Asgn;
    asgn->m_left = left;
    asgn->m_right = right;
    return (Ast*) asgn;
  }

  Ast* new_num(i64 val) {
    Num* num = new Num;
    num->m_val = val;
    return (Ast*) num;
  }

  Ast* new_if(Ast* cond, Ast* then_do, Ast* else_do, Scope* then_scope, Scope* else_scope, Scope* prev_scope) {
    If* aif = new If;
    aif->m_cond_ast = cond;
    aif->m_then_ast = then_do;
    aif->m_else_ast = else_do;
    aif->m_then_scope = then_scope;
    aif->m_else_scope = else_scope;
    aif->m_prev_scope = prev_scope;
    return (Ast*) aif;
  }

  Ast* new_for(Ast* init, Ast* cond, Ast* inc, Ast* todo, Scope* for_scope, Scope* prev_scope) {
    For* afor = new For;
    afor->m_init_ast = init;
    afor->m_inc_ast = inc;
    afor->m_cond_ast = cond;
    afor->m_do_ast = todo;
    afor->m_for_scope = for_scope;
    afor->m_prev_scope = prev_scope;
    return (Ast*) afor;
  }

  Ast* new_ret(Ast* ret) {
    Ret* aret = new Ret;
    aret->m_ret = ret;
    return (Ast*) aret;
  }

  Ast* new_vardef(Symbol* sym, i64 val, u32 intlen) {
    Vardef* vardef = new Vardef;
    vardef->m_sym = sym;
    vardef->m_val = val;
    vardef->m_intlen = intlen;
    return (Ast*) vardef;
  }

  /**
   * Call this function after m_in has been set up
   * Will ignore if any wire in the symbol's bundle is not in m_in
   */
  void dir_input(Symbol* sym, i64 val) {
    if (sym->m_type == NUM) {
      NumSymbol* nsym = (NumSymbol*) sym;
      for (u32 i = 0; i < nsym->m_bundle.size(); ++i) {
        Wire* w = nsym->m_bundle[i];
        if (!mgc.m_in->hasWire(w->m_id)) {
          WARNING("Directory symbol is not in input bundle of circuit.");
          return;
        }
        GASSERT(mgc.m_in->getWire(w->m_id) == w);   // Require one pointer to a wire
        w->m_v = getbit(val, i);
      }
    } else if (sym->m_type == ARRAY) {
      NOT_YET_IMPLEMENTED("dir_input : ARRAY");
    } else {
      WARNING("Invalid input type : " <<  sym->m_type);
    }
  }

}
