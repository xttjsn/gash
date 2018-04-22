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

}
