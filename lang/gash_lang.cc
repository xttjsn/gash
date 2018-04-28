/*
 * gash_lang.cc -- The GCDF compiler of gash
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
#include "circuit.h"
#include "parser.tab.h"

#define FORCE_SYM_CAST(sym, type, nsym) \
    do {                                \
        GASSERT(sym->m_type == type);   \
        nsym = (NumSymbol*)sym;         \
    } while (0)

extern FILE* yyin;

namespace gashlang {

Circuit mgc;
ExeCtx mectx;

void evalast_aop(Aop* aop, Bundle& bret);
void evalast_bop(Bop* bop, Bundle& bret);
void evalast_cop(Cop* cop, Bundle& bret);
void evalast_ref(Ref* ref, Bundle& bret);
void evalast_asgn(Asgn* asgn, Bundle& bret);
void evalast_num(Num* num, Bundle& bret);
void evalast_ret(Ret* ret, Bundle& bret);
void evalast_vdf(Vardef* vdf, Bundle& bret);
void evalast_if(If* aif, Bundle& bret);
void evalast_ifel(Ifel* aifel, Bundle& bret);
void evalast_for(For* afor, Bundle& bret);
void evalast_dir(Dir* dir);

void evalast_n_asgn(Asgn* asgn, i64& ret);
void evalast_n_aop(Aop* aop, i64& ret);
void evalast_n_bop(Bop* bop, i64& ret);
void evalast_n_cop(Cop* cop, i64& ret);
void evalast_n_ref(Ref* ref, i64& ret);
void evalast_n_num(Num* num, i64& ret);

Func* defun(Symbol* sym, Ast* vdf_ast, Ast* do_ast)
{
    Func* func = new Func;

    GASSERT(sym->m_type == FUNC);
    FuncSymbol* fsym = (FuncSymbol*)sym;

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

Ast* new_ref_int(Symbol* sym)
{
    IntRef* ref = new IntRef;
    ref->m_reftype = rINT;
    ref->m_sym = sym;
    return (Ast*)ref;
}

Ast* new_ref_bit(Symbol* sym, Ast* bit_idx_ast)
{
    BitRef* ref = new BitRef;
    ref->m_reftype = rBIT;
    ref->m_sym = sym;
    ref->m_bit_idx_ast = bit_idx_ast;
    return (Ast*)ref;
}

Ast* new_asgn(Ast* left, Ast* right)
{
    Asgn* asgn = new Asgn;
    asgn->m_left = left;
    asgn->m_right = right;
    return (Ast*)asgn;
}

Ast* new_num(i64 val)
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

Ast* new_vdf(Symbol* sym, u32 intlen)
{
    Vardef* vardef = new Vardef;
    NumSymbol* nsym;
    FORCE_SYM_CAST(sym, NUM, nsym);
    nsym->m_len = intlen;
    vardef->m_sym = sym;
    return (Ast*)vardef;
}

/**
 * Call this function after m_in has been set up
 * Will ignore if any wire in the symbol's bundle is not in m_in
 */
Ast* new_dir_input(Symbol* sym, i64 val)
{
    Dir* dir = new Dir;
    dir->m_sym = sym;
    dir->m_val = val;
    return (Ast*)dir;
}

void dir_ip(const char* ip)
{
    strncpy(mectx.m_ip, ip, 16); // Use safe strcpy
}

void dir_port(u32 port)
{
    mectx.m_port = port;
}

void dir_role(RoleType role)
{
    mectx.m_role = role;
}

void evalast(Ast* ast, Bundle& bret)
{
    if (!ast) {
        return;
    }

    switch (ast->m_nodetype) {
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
    }
    case nFOR: {
        For* afor = (For*)ast;
        evalast_for(afor, bret);
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
        evala_ADD(bleft, bright, bret);
        break;
    case AOP_SUB:
        evalast(aop->m_left, bleft);
        evalast(aop->m_right, bright);
        evala_SUB(bleft, bright, bret);
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

void evalast_bop(Bop* bop, Bundle& bret)
{
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
    case BOP_SHL: {
        evalast(bop->m_left, bleft);
        i64 n_shl = evalast_n(bop->m_n_ast);
        evalb_SHL(bleft, n_shl, bret);
    } break;
    case BOP_SHR: {
        evalast(bop->m_left, bleft);
        i64 n_shr = evalast_n(bop->m_n_ast);
        evalb_SHR(bleft, n_shr, bret);
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

void evalast_ref(Ref* ref, Bundle& bret)
{
    NumSymbol* sym;
    FORCE_SYM_CAST(ref->m_sym, NUM, sym);
    if (ref->m_reftype == rINT) {
        bret = sym->m_bundle;
    } else if (ref->m_reftype == rBIT) {
        BitRef* bref = (BitRef*)ref;
        // Evaluate the bit idx ast
        i64 idx = evalast_n(bref->m_bit_idx_ast);
        GASSERT(idx > 0);
        bret.clear();
        bret.add(sym->m_bundle[idx]);
    }
}

void evalast_asgn(Asgn* asgn, Bundle& bret)
{
    Bundle bleft;
    Bundle bright;
    evalast(asgn->m_left, bleft);
    evalast(asgn->m_right, bright);

    // Left and right must have same size
    GASSERT(bleft.size() == bright.size());

    // Copy right to left
    bleft.copyfrom(bright, 0, 0, bright.size());

    // one represents success
    bret = Bundle(onewire());
}

void evalast_num(Num* num, Bundle& bret)
{
    num2bundle(num->m_val, bret);
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
    mgc.m_out = bret;
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
            evalo_if(bcond[0], if_sym->m_bundle, prev_sym->m_bundle,
                new_sym->m_bundle);
        }
    }

    // one represents success
    bret = Bundle(onewire());
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

        // If this symbol was in prev_scope
        if (prev_scope->has_symbol_for_name(sym_name)) {
            // Get the symbol from prev_scope
            NumSymbol* prev_sym;
            FORCE_SYM_CAST(prev_scope->get_symbol_for_name(sym_name), NUM, prev_sym);

            // Create a new symbol
            NumSymbol* new_sym;
            FORCE_SYM_CAST(prev_scope->new_symbol(prev_sym), NUM, new_sym);

            // If this symbol was in else_scope
            if (else_scope->has_symbol_for_name(sym_name)) {
                NumSymbol* else_sym;
                FORCE_SYM_CAST(else_scope->get_symbol_for_name(sym_name), NUM,
                    else_sym);

                evalo_if(bcond[0], if_sym->m_bundle, else_sym->m_bundle,
                    new_sym->m_bundle);

            } else { // If this symbol was not in else_scope

                evalo_if(bcond[0], if_sym->m_bundle, prev_sym->m_bundle,
                    new_sym->m_bundle);
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

        // If this symbol was in prev_scope
        if (prev_scope->has_symbol_for_name(sym_name)) {
            // Get the symbol from prev_scope
            NumSymbol* prev_sym;
            FORCE_SYM_CAST(prev_scope->get_symbol_for_name(sym_name), NUM, prev_sym);

            // If this symbols was NOT in if_scope
            if (!if_scope->has_symbol_for_name(sym_name)) {
                NumSymbol* new_sym;
                FORCE_SYM_CAST(prev_scope->new_symbol(prev_sym), NUM, new_sym);
                evalo_if(bcond[0], else_sym->m_bundle, prev_sym->m_bundle,
                    new_sym->m_bundle);

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
    i64 val = dir->m_val;

    if (sym->m_type == NUM) {
        NumSymbol* nsym = (NumSymbol*)sym;
        for (u32 i = 0; i < nsym->m_bundle.size(); ++i) {
            Wire* w = nsym->m_bundle[i];
            if (!mgc.m_in.hasWire(w->m_id)) {
                WARNING("Directory symbol is not in input bundle of circuit.");
                return;
            }
            GASSERT(mgc.m_in.getWire(w->m_id) == w); // Require one pointer to a wire
            w->m_v = getbit(val, i);
        }
    } else if (sym->m_type == ARRAY) {
        NOT_YET_IMPLEMENTED("dir_input : ARRAY");
    } else {
        WARNING("Invalid input type : " << sym->m_type);
    }
}

i64 evalast_n(Ast* ast)
{
    i64 ret;
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

void evalast_n_asgn(Asgn* asgn, i64& ret)
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

void evalast_n_aop(Aop* aop, i64& ret)
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

void evalast_n_bop(Bop* bop, i64& ret)
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

void evalast_n_cop(Cop* cop, i64& ret)
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

void evalast_n_ref(Ref* ref, i64& ret)
{
    GASSERT(ref->m_reftype == rINT);
    IntRef* iref = (IntRef*)ref;
    Symbol* sym = iref->m_sym;
    GASSERT(sym->m_type == NUM);
    NumSymbol* nsym = (NumSymbol*)sym;
    ret = nsym->m_value;
}

void evalast_n_num(Num* num, i64& ret)
{
    ret = num->m_val;
}

void write_circuit()
{
    mgc.write();
}

void write_data()
{
    mgc.write_input();
}

void cleanup()
{
    // Not yet implemented
}

void yyerror(const char* s, ...)
{
    fprintf(stderr, "%s line:%d, first_column:%d, last_column:%d", s,
        yylloc.first_line, yylloc.first_column, yylloc.last_column);
    exit(-1);
}

void run(ofstream& circ_file,
    ofstream& data_file,
    const char* circ_out,
    const char* data_out,
    const char* input,
    const char* mode)
{
    mgc.set_circ_outstream(circ_file);
    mgc.set_data_outstream(data_file);

    mectx.m_circ_path = new char[strlen(circ_out) + 1];
    strcpy(mectx.m_circ_path, circ_out);
    mectx.m_data_path = new char[strlen(data_out) + 1];
    strcpy(mectx.m_data_path, data_out);

    yyin = fopen(input, "r");

    int parseresult = yyparse();

    if (strcmp(mode, "normal") == 0) {
        exec(mectx);
    } else {
        FATAL("Invalid mode" << mode);
    }
}
}
