/*
 * lang.cpp -- The GCDF compiler of gash
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

#include "lang.hpp"
#include "evaluator.hpp"
#include "garbler.hpp"
#include "garbled_circuit.hpp"
#include "parser.tab.h"

#define FORCE_SYM_CAST(sym, type, nsym)         \
    do {                                        \
        nsym = (NumSymbol*)sym;                 \
    } while (0)

extern FILE* yyin;

GarbledCircuit* mgc;
static Garbler* mgarbler = nullptr;
static Evaluator* mevaluator = nullptr;

void evalast_aop(Aop* aop, Bundle& bret);
void evalast_bop(Bop* bop, Bundle& bret);
void evalast_cop(Cop* cop, Bundle& bret);
void evalast_ref(Ref* ref, Bundle& bret);
Bundle* evalast_ref(Ref* ref);
void evalast_asgn(Asgn* asgn, Bundle& bret);
void evalast_num(Num* num, Bundle& bret);
void evalast_ret(Ret* ret, Bundle& bret);
void evalast_vdf(Vardef* vdf, Bundle& bret);
Bundle* evalast_vdf(Vardef* vdf);
void evalast_if(If* aif, Bundle& bret);
void evalast_ifel(Ifel* aifel, Bundle& bret);
void evalast_for(For* afor, Bundle& bret);
void evalast_dir(Dir* dir);

void evalast_n_asgn(Asgn* asgn, int64_t& ret);
void evalast_n_aop(Aop* aop, int64_t& ret);
void evalast_n_bop(Bop* bop, int64_t& ret);
void evalast_n_cop(Cop* cop, int64_t& ret);
void evalast_n_ref(Ref* ref, int64_t& ret);
void evalast_n_num(Num* num, int64_t& ret);

Func* defun(Symbol* sym, Ast* vdf_ast, Ast* do_ast)
{
    Func* func = new Func;

    GASSERT(sym->m_type == FUNC);
    FuncSymbol* fsym = (FuncSymbol*)sym;

    Ast* curr_vdf_ast = vdf_ast;
    Vardef* vdf;
    while (curr_vdf_ast) {
        vdf = (Vardef*)curr_vdf_ast->m_left;
        GASSERT(vdf->m_nodetype == nVDF);
        vdf->m_isinput = true;
        curr_vdf_ast = curr_vdf_ast->m_right;
    }

    func->m_fsym = fsym;
    func->m_vdf_ast = vdf_ast;
    func->m_do_ast = do_ast;

    return func;
}

Ast* new_ast(Ast* left, Ast* right)
{
    Ast* ast = new Ast;
    REQUIRE_NOT_NULL(ast);
    ast->m_left = left;
    ast->m_right = right;
    return ast;
}

Ast* new_aop(int op, Ast* left, Ast* right)
{
    Aop* aop = new Aop;
    REQUIRE_NOT_NULL(aop);
    aop->m_op = op;
    aop->m_left = left;
    aop->m_right = right;
    return (Ast*)aop;
}

Ast* new_bop(int op, Ast* left, Ast* right)
{
    Bop* bop = new Bop;
    REQUIRE_NOT_NULL(bop);
    bop->m_op = op;
    bop->m_left = left;
    bop->m_right = right;
    return (Ast*)bop;
}

Ast* new_cop(int op, Ast* left, Ast* right)
{
    Cop* cop = new Cop;
    REQUIRE_NOT_NULL(cop);
    cop->m_op = op;
    cop->m_left = left;
    cop->m_right = right;
    return (Ast*)cop;
}

Ast* new_ref_int(Symbol* sym, Scope* scope)
{
    IntRef* ref = new IntRef;
    ref->m_reftype = rINT;
    ref->m_sym = sym;
    ref->m_scope = scope;
    return (Ast*)ref;
}

Ast* new_ref_bit(Symbol* sym, Ast* bit_idx_ast, Scope* scope)
{
    BitRef* ref = new BitRef;
    ref->m_reftype = rBIT;
    ref->m_sym = sym;
    ref->m_bit_idx_ast = bit_idx_ast;
    ref->m_scope = scope;
    return (Ast*)ref;
}

Ast* new_asgn(Ast* left, Ast* right)
{
    Asgn* asgn = new Asgn;
    asgn->m_left = left;
    asgn->m_right = right;
    return (Ast*)asgn;
}

Ast* new_num(int64_t val)
{
    Num* num = new Num;
    num->m_val = val;
    return (Ast*)num;
}

Ast* new_if(Ast* cond, Ast* if_ast, Scope* if_scope, Scope* prev_scope)
{
    If* aif = new If;
    aif->m_cond_ast = cond;
    aif->m_if_ast = if_ast;
    aif->m_if_scope = if_scope;
    aif->m_prev_scope = prev_scope;
    return (Ast*)aif;
}

Ast* new_ifelse(Ast* cond,
                Ast* if_ast,
                Ast* else_ast,
                Scope* if_scope,
                Scope* else_scope,
                Scope* prev_scope)
{
    Ifel* aifel = new Ifel;
    aifel->m_cond_ast = cond;
    aifel->m_if_ast = if_ast;
    aifel->m_else_ast = else_ast;
    aifel->m_if_scope = if_scope;
    aifel->m_else_scope = else_scope;
    aifel->m_prev_scope = prev_scope;
    return (Ast*)aifel;
}

Ast* new_for(Ast* init,
             Ast* cond,
             Ast* inc,
             Ast* todo,
             Scope* for_scope,
             Scope* prev_scope)
{
    For* afor = new For;
    afor->m_init_ast = init;
    afor->m_inc_ast = inc;
    afor->m_cond_ast = cond;
    afor->m_do_ast = todo;
    afor->m_for_scope = for_scope;
    afor->m_prev_scope = prev_scope;
    return (Ast*)afor;
}

Ast* new_ret(Ast* ret)
{
    Ret* aret = new Ret;
    aret->m_ret = ret;
    return (Ast*)aret;
}

Ast* new_vdf(Symbol* sym, uint32_t intlen)
{
    Vardef* vardef = new Vardef;
    NumSymbol* nsym;
    FORCE_SYM_CAST(sym, NUM, nsym);
    nsym->m_len = intlen;
    for (int i = 0; i < intlen; ++i) {
        nsym->m_bundle.push_back(mgc->nextwi());
    }
    vardef->m_sym = sym;
    return (Ast*)vardef;
}

/**
 * Call this function after m_in has been set up
 * Will ignore if any wire in the symbol's bundle is not in m_in
 */
Ast* new_dir_input(Symbol* sym, int64_t val)
{
    Dir* dir = new Dir;
    dir->m_sym = sym;
    dir->m_val = val;
    return (Ast*)dir;
}

void dir_ip(const char* ip)
{
    mevaluator->m_peer_ip = string(ip);
}

void evalast(Ast* ast, Bundle& bret)
{
    if (!ast) {
        return;
    }

    switch (ast->m_nodetype) {
    case nFUNC: {
        Func* func = (Func*)ast;
        Bundle b1;
        Bundle b2;
        evalast(func->m_vdf_ast, b1);
        evalast(func->m_do_ast, b2);
        break;
    }
    case nLIST: {
        Bundle b1;
        Bundle b2;
        evalast(ast->m_left, b1);
        evalast(ast->m_right, b2);
        break;
    }
    case nAOP: {
        Aop* aop = (Aop*)ast;
        evalast_aop(aop, bret);
    } break;
    case nBOP: {
        Bop* bop = (Bop*)ast;
        evalast_bop(bop, bret);
    } break;
    case nCOP: {
        Cop* cop = (Cop*)ast;
        evalast_cop(cop, bret);
    } break;
    case nNAME: {
        FATAL("nNAME should never be evaluated, REF should have handled it.");
    } break;
    case nREF: {
        Ref* ref = (Ref*)ast;
        evalast_ref(ref, bret);
    } break;
    case nASGN: {
        Asgn* asgn = (Asgn*)ast;
        evalast_asgn(asgn, bret);
    } break;
    case nNUM: {
        Num* num = (Num*)ast;
        evalast_num(num, bret);
    } break;
    case nRET: {
        Ret* ret = (Ret*)ast;
        evalast_ret(ret, bret);
    } break;
    case nVDF: {
        Vardef* vdf = (Vardef*)ast;
        evalast_vdf(vdf, bret);
    } break;
    case nIF: {
        If* aif = (If*)ast;
        evalast_if(aif, bret);
    } break;
    case nIFEL: {
        Ifel* aifel = (Ifel*)ast;
        evalast_ifel(aifel, bret);
    } break;
    case nFOR: {
        For* afor = (For*)ast;
        evalast_for(afor, bret);
    } break;
    case nDIR: {
        Dir* dir = (Dir*)ast;
        evalast_dir(dir);
        break;
    }
    default:
        FATAL("Unsupported nodetype : " << ast->m_nodetype);
        break;
    }
}

/**
 * This is the evalast function used for left side arguments (assignable reference)
 *
 * @param ast
 *
 * @return The pointer to the assignable bundle
 */
Bundle* evalast(Ast* ast) {

    if (!ast) {
        return NULL;
    }

    switch (ast->m_nodetype) {
    case nNAME: {
        FATAL("nNAME should never be evaluated, REF should have handled it.");
    } break;
    case nREF: {
        Ref* ref = (Ref*)ast;
        return evalast_ref(ref);
    } break;
    case nVDF: {
        Vardef* vdf = (Vardef*)ast;
        return evalast_vdf(vdf);
    } break;
    default:
        FATAL("Unsupported nodetype : " << ast->m_nodetype);
        break;
    }

}

void evalast_aop(Aop* aop, Bundle& bret)
{
    Bundle bleft;
    Bundle bright;
    switch (aop->m_op) {
    case AOP_PLUS:
        evalast(aop->m_left, bleft);
        evalast(aop->m_right, bright);
        bret= *(mgc->evala_ADD(bleft, bright));
        break;
    case AOP_SUB:
        evalast(aop->m_left, bleft);
        evalast(aop->m_right, bright);
        bret = *(mgc->evala_SUB(bleft, bright));
        break;
    case AOP_UMINUS:
        evalast(aop->m_left, bleft);
        bret = *(mgc->evala_UMINUS(bleft));
        break;
    case AOP_MUL:
        evalast(aop->m_left, bleft);
        evalast(aop->m_right, bright);
        bret = *(mgc->evala_MUL(bleft, bright));
        break;
    case AOP_DIV:
        evalast(aop->m_left, bleft);
        evalast(aop->m_right, bright);
        bret = *(mgc->evala_DIV(bleft, bright));
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

void evalast_bop(Bop* bop, Bundle& bret)
{
    Bundle bleft;
    Bundle bright;
    switch (bop->m_op) {
    case BOP_OR:
        evalast(bop->m_left, bleft);
        evalast(bop->m_right, bright);
        bret = *(mgc->evalb_OR(bleft, bright));
        break;
    case BOP_AND:
        evalast(bop->m_left, bleft);
        evalast(bop->m_right, bright);
        bret = *(mgc->evalb_AND(bleft, bright));
        break;
    case BOP_XOR:
        evalast(bop->m_left, bleft);
        evalast(bop->m_right, bright);
        bret = *(mgc->evalb_XOR(bleft, bright));
        break;
    case BOP_INV:
        evalast(bop->m_left, bleft);
        bret = *(mgc->evalb_INV(bleft));
        break;
    case BOP_SHL: {
        evalast(bop->m_left, bleft);
        int64_t n_shl = evalast_n(bop->m_n_ast);
        bret = *(mgc->evalb_SHL(bleft, n_shl));
    } break;
    case BOP_SHR: {
        evalast(bop->m_left, bleft);
        int64_t n_shr = evalast_n(bop->m_n_ast);
        bret = *(mgc->evalb_SHR(bleft, n_shr));
    } break;
    }
}

void evalast_cop(Cop* cop, Bundle& bret)
{
    Bundle bleft;
    Bundle bright;
    switch (cop->m_op) {
    case COP_LA:
        evalast(cop->m_left, bleft);
        evalast(cop->m_right, bright);
        bret = Bundle({mgc->evalc_LA(bleft, bright)});
        break;
    case COP_LE:
        evalast(cop->m_left, bleft);
        evalast(cop->m_right, bright);
        bret = Bundle({mgc->evalc_LE(bleft, bright)});
        break;
    case COP_LAE:
        evalast(cop->m_left, bleft);
        evalast(cop->m_right, bright);
        bret = Bundle({mgc->evalc_LAE(bleft, bright)});
        break;
    case COP_LEE:
        evalast(cop->m_left, bleft);
        evalast(cop->m_right, bright);
        bret = Bundle({mgc->evalc_LEE(bleft, bright)});
        break;
    case COP_EQ:
        evalast(cop->m_left, bleft);
        evalast(cop->m_right, bright);
        bret = Bundle({mgc->evalc_EQ(bleft, bright)});
        break;
    case COP_NEQ:
        evalast(cop->m_left, bleft);
        evalast(cop->m_right, bright);
        bret = Bundle({mgc->evalc_NEQ(bleft, bright)});
        break;
    }
}

void evalast_ref(Ref* ref, Bundle& bret)
{
    /**
     * The bundle attached to the symbol might already
     * be an old version. So we should get the newest version
     * of the same symbol
     *
     */

    Symbol* new_sym = ref->m_scope->get_symbol_for_name(ref->m_sym->m_name);
    NumSymbol* sym;
    FORCE_SYM_CAST(new_sym, NUM, sym);
    if (ref->m_reftype == rINT) {
        bret = sym->m_bundle;
    } else if (ref->m_reftype == rBIT) {
        BitRef* bref = (BitRef*)ref;
        // Evaluate the bit idx ast
        int64_t idx = evalast_n(bref->m_bit_idx_ast);
        GASSERT(idx > 0);
        bret.clear();
        bret.push_back(sym->m_bundle[idx]);
    }
}

/**
 * The assignable version of evalast_ref
 * Will create a new symbol if *this* scope does not have the symbol
 * Will not consider whether any of the ancestors has the symbol
 *
 * @param ref
 *
 * @return
 */
Bundle* evalast_ref(Ref* ref) {

    /**
     * If the current scope does not has the symbol, create one with
     * uninitialized values and wires (i.e. new wires).
     * Otherwise, just use the old symbol
     *
     */

    NumSymbol* nsym;

    if (!ref->m_scope->has_symbol_for_name(ref->m_sym->m_name)) {
        // Get the old symbol from ancestor scopes
        Symbol* old_sym = ref->m_scope->get_symbol_for_name(ref->m_sym->m_name);

        Symbol* new_sym = ref->m_scope->new_symbol(old_sym);
        FORCE_SYM_CAST(new_sym, NUM, nsym);

    } else {

        Symbol* sym = ref->m_scope->get_symbol_for_name(ref->m_sym->m_name);
        FORCE_SYM_CAST(sym, NUM, nsym);
    }

    if (ref->m_reftype == rINT) {
        return &nsym->m_bundle;
    } else {
        FATAL("Not yet implemented");
    }
}

void evalast_asgn(Asgn* asgn, Bundle& bret)
{
    Bundle* bleft;
    Bundle bright;

    // Use the assignable evalast to get the assignable bundle
    // Otherwise we will only be able to get a copy
    bleft = evalast(asgn->m_left);
    evalast(asgn->m_right, bright);

    if (bleft->m_isconst) {
        FATAL("Left side bundle cannot be constant.");
    }

    if (bright.m_isconst) {

        // Require the size of the right bundle larger than or equal to that of the left
        // bundle
        GASSERT(bleft->size() <= bright.size());

        // Copy right to left, using only the size of the left bundle
        bleft->copyfrom(bright, 0, 0, bleft->size());
    } else {

        // Left and right must have same size
        GASSERT(bleft->size() == bright.size());

        // Copy right to left
        bleft->copyfrom(bright, 0, 0, bright.size());
    }

    // one represents success
    bret = Bundle(mgc->onewi());
}

void evalast_num(Num* num, Bundle& bret)
{
    bret = *(mgc->num2bundle_n(num->m_val, 64));
}

/**
 * TODO: Have each func class have a return symbol,
 * and assign the result of evalast to the return symbol
 * only.
 *
 */
void evalast_ret(Ret* ret, Bundle& bret)
{
    evalast(ret->m_ret, bret);
    for (int i = 0; i < bret.size(); ++i) {
        mgc->m_out_id_vec.push_back(bret[i]->m_wire->m_id);
    }
}

void evalast_vdf(Vardef* vdf, Bundle& bret)
{
    // Num symbol: return its bundle
    if (vdf->m_sym->m_type == NUM) {
        NumSymbol* sym_n = (NumSymbol*)vdf->m_sym;
        bret = sym_n->m_bundle;
    } else if (vdf->m_sym->m_type == ARRAY) { // Array symbol: return all its bundle as a single one
        ArraySymbol* sym_a = (ArraySymbol*)vdf->m_sym;
        sym_a->get_bundle(bret);
    } else {
        FATAL("Invalid symbol type" << vdf->m_sym->m_type);
    }
    
    if (vdf->m_isinput) {
        for (int i = 0; i < bret.size(); ++i) {
            mgc->m_in_id_set.insert(bret[i]->m_wire->m_id);
        }
    }
}

Bundle* evalast_vdf(Vardef* vdf) {

    // Num symbol: return its bundle
    if (vdf->m_sym->m_type == NUM) {

        NumSymbol* sym_n = (NumSymbol*)vdf->m_sym;

        return &sym_n->m_bundle;

    } else if (vdf->m_sym->m_type == ARRAY) { // Array symbol: return all its bundle as a single one
        FATAL("Not yet implemented");
    } else {
        FATAL("Invalid symbol type" << vdf->m_sym->m_type);
    }

}

void evalast_if(If* aif, Bundle& bret)
{
    Bundle bcond;
    Bundle bif;
    Scope* if_scope = aif->m_if_scope;
    Scope* prev_scope = aif->m_prev_scope;

    // Evaluate cond
    evalast(aif->m_cond_ast, bcond);

    // Condition bundle should have size 1
    GASSERT(bcond.size() == 1);

    // Evaluate if statement
    evalast(aif->m_if_ast, bif);

    // For each symbol in if_scope that's also in prev_scope, create a new symbol
    // in prev_scope
    // And copy the bundle of either if_scope's symbol or prev_scope's symbol to
    // the new symbol,
    // conditioned on cond.
    for (auto it = if_scope->m_symbols.begin(); it != if_scope->m_symbols.end();
         ++it) {
        string sym_name = it->first;
        NumSymbol* if_sym;
        FORCE_SYM_CAST(if_scope->get_symbol_for_name(sym_name), NUM, if_sym);

        // If this symbol was in prev_scope or its ancestor
        if (prev_scope->has_symbol_for_name(sym_name)) {
            NumSymbol* prev_sym;
            FORCE_SYM_CAST(prev_scope->get_symbol_for_name(sym_name), NUM, prev_sym);

            // Create a new symbol
            NumSymbol* new_sym;
            FORCE_SYM_CAST(prev_scope->new_symbol(prev_sym), NUM, new_sym);

            // Assign bundle conditioned on bcond
            new_sym->m_bundle = *(mgc->evalo_if(bcond[0], if_sym->m_bundle, prev_sym->m_bundle));
        }
    }

    // one represents success
    bret = Bundle(mgc->onewi());
}

void evalast_ifel(Ifel* aifel, Bundle& bret)
{
    Bundle bcond;
    Bundle bif;
    Bundle belse;
    Scope* if_scope = aifel->m_if_scope;
    Scope* else_scope = aifel->m_else_scope;
    Scope* prev_scope = aifel->m_prev_scope;

    // Evaluate cond
    evalast(aifel->m_cond_ast, bcond);

    // Condition bundle should have size 1
    GASSERT(bcond.size() == 1);

    // Evaluate if statement
    evalast(aifel->m_if_ast, bif);

    // Evaluate else statement
    evalast(aifel->m_else_ast, belse);

    // For each symbol in if_scope, else_scope and prev_scope, create a new symbol
    // in prev_scope,
    // and copy the bundle of either if_scope's symbol or else_scope's symbol to
    // the new symbol,
    // conditioned on cond.
    for (auto it = if_scope->m_symbols.begin(); it != if_scope->m_symbols.end();
         ++it) {
        string sym_name = it->first;
        NumSymbol* if_sym;
        FORCE_SYM_CAST(if_scope->get_symbol_for_name(sym_name), NUM, if_sym);

        Scope* pscope;       // The pointer to an ancestor scope that has the symbol
        if (if_scope->get_ancestor_scope_that_has_symbol(sym_name, pscope)) {
            NumSymbol* p_sym;
            FORCE_SYM_CAST(pscope->get_symbol_for_name(sym_name), NUM, p_sym);

            NumSymbol* new_sym;
            FORCE_SYM_CAST(pscope->new_symbol(p_sym), NUM, new_sym);

            if (else_scope->has_symbol_for_name(sym_name)) {
                NumSymbol* else_sym;
                FORCE_SYM_CAST(else_scope->get_symbol_for_name(sym_name), NUM, else_sym);

                new_sym->m_bundle = *(mgc->evalo_if(bcond[0], if_sym->m_bundle, else_sym->m_bundle));
            } else {

                new_sym->m_bundle = *(mgc->evalo_if(bcond[0], if_sym->m_bundle, p_sym->m_bundle));
            }

        }

    }

    // Now that we handled symbols in if x else x prev, if x prev, we should
    // handle symbols
    // in else x prev
    for (auto it = else_scope->m_symbols.begin();
         it != else_scope->m_symbols.end(); ++it) {
        string sym_name = it->first;
        NumSymbol* else_sym;
        FORCE_SYM_CAST(else_scope->get_symbol_for_name(sym_name), NUM, else_sym);

        Scope* pscope;
        if (else_scope->get_ancestor_scope_that_has_symbol(sym_name, pscope)) {

            NumSymbol* p_sym;
            FORCE_SYM_CAST(pscope->get_symbol_for_name(sym_name), NUM, p_sym);

            // If this symbols was NOT in if_scope
            if (!if_scope->has_symbol_for_name(sym_name)) {
                NumSymbol* new_sym;
                FORCE_SYM_CAST(pscope->new_symbol(p_sym), NUM, new_sym);
                new_sym->m_bundle = *(mgc->evalo_if(bcond[0], else_sym->m_bundle, p_sym->m_bundle));

            } else {
                // Already handled in last for loop
                // Do nothing
            }
        }
    }
}

void evalast_for(For* afor, Bundle& bret)
{
    // Evaluate init ast in numeric mode. All bundles will be ignored
    evalast_n(afor->m_init_ast);

    // Execute the for loop in a while loop
    while (evalast_n(afor->m_cond_ast)) {
        // Evaluate do in normal mode
        evalast(afor->m_do_ast, bret);

        // Evaluate increment ast in numeric mode
        evalast_n(afor->m_inc_ast);
    }
}

void evalast_dir(Dir* dir)
{
    Symbol* sym = dir->m_sym;
    int64_t val = dir->m_val;

    if (sym->m_type == NUM) {

        NumSymbol*          nsym = (NumSymbol*)sym;
        WI*                 w;
        WI*                 w_dup;
        uint32_t            id;
        uint32_t            id_dup;

        /**
         * For each wire in the bundle, check to see if it's in the input bundle
         * of the circuit. If not, then raise warning. If so, then set the value
         * of the wire. Additionally, check whether this wire has invert duplicate.
         * If so, make sure that the invert duplicate gets the opposite value.
         *
         */
        for (uint32_t i = 0; i < nsym->m_bundle.size(); ++i) {
            WI* w = nsym->m_bundle[i];
            id = w->m_wire->m_id;
            if (mgc->m_in_id_set.find(id) == mgc->m_in_id_set.end()) {
                WARNING("Directory symbol is not in input bundle of circuit.");
                return;
            }
            GASSERT(mgc->get_wi(id) == w); // Require one pointer to a wire
            w->m_wire->m_val = getbit(val, i);

            if (mgc->m_wi_inv_map.find(id) != mgc->m_wi_inv_map.end()) {
                // Found input duplicate
                id_dup = mgc->m_wi_inv_map.find(id)->second;
                if (mgc->m_in_id_set.find(id_dup) == mgc->m_in_id_set.end()) {
                    FATAL("An input wire's invert duplicate is not in the input bundle of circuit.");
                }
                
                w_dup = mgc->get_wi(id_dup);
                w_dup->m_wire->m_val = getbit(val, i) ^ 1;
            }
        }

    } else if (sym->m_type == ARRAY) {
        NOT_YET_IMPLEMENTED("dir_input : ARRAY");
    } else {
        WARNING("Invalid input type : " << sym->m_type);
    }
}

int64_t evalast_n(Ast* ast)
{
    int64_t ret;
    switch (ast->m_nodetype) {
    case nASGN: {
        Asgn* asgn = (Asgn*)ast;
        evalast_n_asgn(asgn, ret);
        break;
    }
    case nAOP: {
        Aop* aop = (Aop*)ast;
        evalast_n_aop(aop, ret);
        break;
    }
    case nBOP: {
        Bop* bop = (Bop*)ast;
        evalast_n_bop(bop, ret);
        break;
    }
    case nCOP: {
        Cop* cop = (Cop*)ast;
        evalast_n_cop(cop, ret);
        break;
    }
    case nREF: {
        Ref* ref = (Ref*)ast;
        evalast_n_ref(ref, ret);
        break;
    }
    case nNUM: {
        Num* num = (Num*)ast;
        evalast_n_num(num, ret);
        break;
    }
    default: {
        FATAL("Unsupported evalast_n for type : " << ast->m_nodetype);
    }
    }

    return ret;
}

void evalast_n_asgn(Asgn* asgn, int64_t& ret)
{
    GASSERT(asgn->m_left->m_nodetype == nREF);
    Ref* ref = (Ref*)asgn->m_left;
    GASSERT(ref->m_reftype == rINT);
    IntRef* iref = (IntRef*)ref;
    Symbol* sym = iref->m_sym;
    GASSERT(sym->m_type == NUM);
    NumSymbol* nsym = (NumSymbol*)sym;
    nsym->m_value = evalast_n(asgn->m_right);
    ret = 1;
}

void evalast_n_aop(Aop* aop, int64_t& ret)
{
    switch (aop->m_op) {
    case AOP_PLUS:
        ret = evalast_n(aop->m_left) + evalast_n(aop->m_right);
        break;
    case AOP_SUB:
        ret = evalast_n(aop->m_left) - evalast_n(aop->m_right);
        break;
    case AOP_MUL:
        ret = evalast_n(aop->m_left) * evalast_n(aop->m_right);
        break;
    case AOP_DIV:
        ret = evalast_n(aop->m_left) / evalast_n(aop->m_right);
        break;
    default:
        FATAL("Unsupporte evalast_n_aop for type : " << aop->m_op);
    }
}

void evalast_n_bop(Bop* bop, int64_t& ret)
{
    switch (bop->m_op) {
    case BOP_AND:
        ret = evalast_n(bop->m_left) & evalast_n(bop->m_right);
        break;
    case BOP_OR:
        ret = evalast_n(bop->m_left) | evalast_n(bop->m_right);
        break;
    case BOP_XOR:
        ret = evalast_n(bop->m_left) ^ evalast_n(bop->m_right);
        break;
    case BOP_SHL:
        ret = evalast_n(bop->m_left) << evalast_n(bop->m_right);
        break;
    case BOP_SHR:
        ret = evalast_n(bop->m_left) >> evalast_n(bop->m_right);
        break;
    default:
        FATAL("Unsupporte evalast_n_bop for type : " << bop->m_op);
    }
}

void evalast_n_cop(Cop* cop, int64_t& ret)
{
    switch (cop->m_op) {
    case COP_LA:
        ret = evalast_n(cop->m_left) > evalast_n(cop->m_right);
        break;
    case COP_LE:
        ret = evalast_n(cop->m_left) < evalast_n(cop->m_right);
        break;
    case COP_EQ:
        ret = evalast_n(cop->m_left) == evalast_n(cop->m_right);
        break;
    case COP_LAE:
        ret = evalast_n(cop->m_left) >= evalast_n(cop->m_right);
        break;
    case COP_LEE:
        ret = evalast_n(cop->m_left) <= evalast_n(cop->m_right);
        break;
    case COP_NEQ:
        ret = evalast_n(cop->m_left) != evalast_n(cop->m_right);
        break;
    default:
        FATAL("Unsupporte evalast_n_cop for type : " << cop->m_op);
    }
}

void evalast_n_ref(Ref* ref, int64_t& ret)
{
    GASSERT(ref->m_reftype == rINT);
    IntRef* iref = (IntRef*)ref;
    Symbol* sym = iref->m_sym;
    GASSERT(sym->m_type == NUM);
    NumSymbol* nsym = (NumSymbol*)sym;
    ret = nsym->m_value;
}

void evalast_n_num(Num* num, int64_t& ret)
{
    ret = num->m_val;
}

void set_garbler(Garbler* g) {
    mgarbler = g;
    mgc = &g->m_gc;
}

void set_evaluator(Evaluator* e) {
    mevaluator = e;
    mgc = &e->m_gc;
}

void yyerror(const char* s, ...)
{
    fprintf(stderr, "%s line:%d, first_column:%d, last_column:%d", s,
            yylloc.first_line, yylloc.first_column, yylloc.last_column);
    exit(-1);
}

void cleanup() {
    // Not yet implementedl
}

void parse_clean()
{

    ScopeStack::instance().clear();
    get_symbol_store().clear();
    mgc = nullptr;
}
