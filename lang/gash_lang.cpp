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
  static ExeCtx mectx;

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

  void dir_ip(const char* ip) {
    strcpy_s(mectx.m_ip, 16, ip);   // Use safe strcpy
  }

  void dir_port(u32 port) {
    mectx.m_port = port;
  }

  void dir_role(RoleType role) {
    mectx.m_role = role;
  }

  void evalast(Ast* ast, Bundle& bret) {
    REQUIRE_NOT_NULL(ast);

    switch (ast->m_nodetype) {
    case nAOP:
      Aop* aop = (Aop*) ast;
      evalast_aop(aop, bret);
      break;
    case nBOP:
      Bop* bop = (Bop*) ast;
      evalast_bop(bop, bret);
      break;
    case nCOP:
      Cop* cop = (Cop*) ast;
      evalast_cop(cop, bret);
      break;
    case nNAME:
      FATAL("nNAME should never be evaluated, REF should have handled it.");
      break;
    case nREF:
      Ref* ref = (Ref*) ast;
      evalast_ref(ref, bret);
      break;
    case nASGN:
      Asgn* asgn = (Asgn*) ast;
      evalast_asgn(asgn, bret);
      break;
    case nNUM:
      Num* num = (Num*) ast;
      evalast_num(num, bret);
      break;
    case nRET:
      break;
    case nVARD:
      break;
    case nIF:
      break;
    case nFOR:
      break;
    case nDIR:
      break;
    }
  }

  void evalast_aop(Aop* aop, Bundle& bret) {
      Bundle bleft;
      Bundle bright;
      switch (aop->m_op) {
      case AOP_PLUS:
        evalast(aop->m_left, bleft);
        evalast(aop->m_right, bright);
        evala_ADD(bleft, bright, bret);
        break;
      case AOP_MINUS:
        evalast(aop->m_left, bleft);
        evalast(aop->m_right, bright);
        evala_MINUS(bleft, bright, bret);
        break;
      case AOP_UMINUS:
        evalast(aop->m_left, bleft);
        evala_UMINUS(bleft, bret);
        break;
      case AOP_MUL:
        evalast(aop->m_left, bleft);
        evalast(aop->m_right, bright);
        evala_MUL(bleft, bright, bret);
        break;
      case AOP_DIV:
        evalast(aop->m_left, bleft);
        evalast(aop->m_right, bright);
        evala_DIV(bleft, bright, bret);
        break;
#ifdef __ADV_ARITH__
      case AOP_SQR:
        NOT_YET_IMPLEMENTED("evalc : AOP_SQR");
        break;
      case AOP_SQRT:
        NOT_YET_IMPLEMENTED("evalc : AOP_SQRT");
        break;
#ifdef __PWS_LIN_APPRX__
      case AOP_LOG2:
        NOT_YET_IMPLEMENTED("evalc : AOP_LOG10");
        break;
      case AOP_LOG10:
        NOT_YET_IMPLEMENTED("evalc : AOP_LOG10");
        break;
#endif
#endif
      }
  }

  void evalast_bop(Bop* bop, Bundle& bret) {
    Bundle bleft;
    Bundle bright;
    switch (bop->m_op) {
    case BOP_OR:
      evalast(bop->m_left, bleft);
      evalast(bop->m_right, bright);
      evalb_OR(bleft, bright, bret);
      break;
    case BOP_AND:
      evalast(bop->m_left, bleft);
      evalast(bop->m_right, bright);
      evalb_AND(bleft, bright, bret);
      break;
    case BOP_XOR:
      evalast(bop->m_left, bleft);
      evalast(bop->m_right, bright);
      evalb_XOR(bleft, bright, bret);
      break;
    case BOP_INV:
      evalast(bop->m_left, bleft);
      evalb_INV(bleft, bret);
      break;
    case BOP_SHL:
      evalast(bop->m_left, bleft);
      evalb_SHL(bleft, bop->m_n, bret);
      break;
    case BOP_SHR:
      evalast(bop->m_left, bleft);
      evalb_SHR(bleft, bop->m_n, bret);
      break;
    }
  }

  void evalast_cop(Cop* cop, Bundle& bret) {
    Bundle bleft;
    Bundle bright;
    switch (cop->m_op) {
    case COP_LA:
      evalast(cop->m_left, bleft);
      evalast(cop->m_right, bright);
      evalc_LA(bleft, bright, bret);
      break;
    case COP_LE:
      evalast(cop->m_left, bleft);
      evalast(cop->m_right, bright);
      evalc_LE(bleft, bright, bret);
      break;
    case COP_LAE:
      evalast(cop->m_left, bleft);
      evalast(cop->m_right, bright);
      evalc_LAE(bleft, bright, bret);
      break;
    case COP_LEE:
      evalast(cop->m_left, bleft);
      evalast(cop->m_right, bright);
      evalc_LEE(bleft, bright, bret);
      break;
    case COP_EQ:
      evalast(cop->m_left, bleft);
      evalast(cop->m_right, bright);
      evalc_EQ(bleft, bright, bret);
      break;
    case COP_NEQ:
      evalast(cop->m_left, bleft);
      evalast(cop->m_right, bright);
      evalc_NEQ(bleft, bright, bret);
      break;
    }
  }

  void evalast_ref(Ref* ref, Bundle& bret) {
    if (ref->m_reftype == rNORMAL) {
      bret = ref->m_sym->m_bundle;
    } else if (ref->m_reftype == rBIT) {
      bret.clear();
      bret.add(ref->m_sym->m_bundle[ref->m_bit_idx]);
    }
  }

  void evalast_asgn(Asgn* asgn, Bundle& bret) {
    Bundle bleft;
    Bundle bright;
    evalast(asgn->m_left, bleft);
    evalast(asgn->m_right, bright);
    GASSERT(bleft.size() == bright.size());   // Must have same size
    bleft.copyfrom(bright, 0, 0, bright.size());
    bret = Bundle(ONE);
  }

  void evalast_num(Num* num, Bundle& bret) {
    num2bundle(num->m_val, bret);
  }
}
