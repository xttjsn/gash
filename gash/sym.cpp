//
//  sym.cpp
//  gash
//
//  Created by Xiaoting Tang on 6/18/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#include "sym.hpp"
#include "circuit.hpp"

extern Circuit* mgc;

NumSymbol::NumSymbol(string name, uint32_t version)
: Symbol(name, version)
{
    m_type = NUM;
}

NumSymbol::NumSymbol(NumSymbol& rhs)
: Symbol(rhs.m_name, rhs.m_version)
{
    for (int i = 0; i < rhs.m_bundle.size(); ++i) {
        m_bundle.push_back(mgc->nextwi());
    }
}

ArraySymbol::ArraySymbol(string name, uint32_t version)
: Symbol(name, version)
{
    m_type = ARRAY;
}

ArraySymbol::ArraySymbol(ArraySymbol& rhs)
: Symbol(rhs.m_name, rhs.m_version)
{
    Bundle b;
    for (int i = 0; i < rhs.m_bundles.size(); ++i) {
        for (int j = 0; j < rhs.m_len; ++j) {
            b.push_back(mgc->nextwi());
        }
        m_bundles.push_back(b);
    }
}

void ArraySymbol::get_bundle(Bundle& bret)
{
    NOT_YET_IMPLEMENTED("ArraySymbol::get_bundle");
}

FuncSymbol::FuncSymbol(string name, uint32_t version)
: Symbol(name, version)
{
    m_type = FUNC;
}

FuncSymbol::FuncSymbol(FuncSymbol& rhs)
: Symbol(rhs.m_name, rhs.m_version)
{
    m_func = rhs.m_func;
}

/**
 * Symbol Store
 */

bool SymbolStore::has_symbol_for_name(string name)
{
    return m_symbols.find(name) != m_symbols.end();
}

void SymbolStore::require_has_symbol_for_name(string name)
{
    if (!has_symbol_for_name(name))
    FATAL("Does not has symbol " << name);
}

bool SymbolStore::version_consistency_check()
{
    for (auto it = m_symbols.begin(); it != m_symbols.end(); ++it) {
        uint32_t version = 0;
        for (auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
            if ((*jt)->m_version != version++)
            return false;
        }
    }
    return true;
}

bool SymbolStore::version_consistency_check_for_name(string name)
{
    require_has_symbol_for_name(name);
    auto it = this->m_symbols.find(name);
    uint32_t version = 0;
    for (auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
        if ((*jt)->m_version != version++)
        return false;
    }
    return true;
}

uint32_t SymbolStore::get_newest_version_for_name(string name)
{
    require_has_symbol_for_name(name);
    return m_symbols.find(name)->second.back()->m_version;
}

Symbol* SymbolStore::get_symbol_for_name(string name)
{
    require_has_symbol_for_name(name);
    return m_symbols.find(name)->second.back();
}

Symbol* SymbolStore::new_symbol(string name, SymbolType type)
{
    Symbol* sym;
    if (has_symbol_for_name(name)) {
        Symbol* sym_old = get_symbol_for_name(name);
        if (type != sym_old->m_type) {
            FATAL("Symbol "
                  << name << " already has type " << type
                  << ", but now you want a new symbol with the same name of type "
                  << type);
        }
        switch (type) {
                case NUM:
                sym = (Symbol*)new NumSymbol(*static_cast<NumSymbol*>(sym_old));
                break;
                case ARRAY:
                sym = (Symbol*)new ArraySymbol(*static_cast<ArraySymbol*>(sym_old));
                break;
                case FUNC:
                sym = (Symbol*)new FuncSymbol(*static_cast<FuncSymbol*>(sym_old));
                break;
            default:
                FATAL("Cannot create symbol of NONE type.");
        }
        sym->gets_older();
        m_symbols.find(sym->m_name)->second.push_back(sym);
        
    } else {
        // Creating a new list of symbols associated with the name, starting from
        // version 0.
        switch (type) {
                case NUM:
                sym = (Symbol*)new NumSymbol(name, 0);
                break;
                case ARRAY:
                sym = (Symbol*)new ArraySymbol(name, 0);
                break;
                case FUNC:
                sym = (Symbol*)new FuncSymbol(name, 0);
                break;
            default:
                FATAL("Cannot create symbol of NONE type.");
        }
        vector<Symbol*> symbol_list({ sym });
        m_symbols.insert(make_pair(name, symbol_list));
    }
    
    // Return the newly created symbol
    return sym;
}

Symbol* SymbolStore::new_symbol(Symbol* sym_old)
{
    // If the old symbol is not even in the store.
    if (!has_symbol_for_name(sym_old->m_name)) {
        FATAL(
              "Old symbol is not in symbol store. Creating raw symbol by hand is "
              "forbidden, use new_symbol");
    }
    Symbol* sym;
    switch (sym_old->m_type) {
            case NUM:
            sym = (Symbol*)new NumSymbol(*static_cast<NumSymbol*>(sym_old));
            break;
            case ARRAY:
            sym = (Symbol*)new ArraySymbol(*static_cast<ArraySymbol*>(sym_old));
            break;
            case FUNC:
            sym = (Symbol*)new FuncSymbol(*static_cast<FuncSymbol*>(sym_old));
            break;
        default:
            FATAL("Invalid symbol type" << sym_old->m_type);
            break;
    }
    sym->gets_older();
    m_symbols.find(sym->m_name)->second.push_back(sym);
    
    return sym;
}

void SymbolStore::clear()
{
    for (auto it = m_symbols.begin(); it != m_symbols.end(); ++it) {
        vector<Symbol*>& symvec = it->second;
        for (auto sit = symvec.begin(); sit != symvec.end(); ++sit) {
            delete *sit;
        }
    }
    m_symbols.clear();
}

/**
 * Scope
 */

bool Scope::has_symbol_for_name(string name)
{
    return m_symbols.find(name) != m_symbols.end();
}

void Scope::add_symbol(Symbol* sym)
{
    if (has_symbol_for_name(sym->m_name)) {
        m_symbols.find(sym->m_name)->second.push_back(sym);
    } else {
        vector<Symbol*> sym_list({ sym });
        m_symbols.insert(make_pair(sym->m_name, sym_list));
    }
}

Symbol* Scope::new_symbol(string name, SymbolType type)
{
    Symbol* sym = get_symbol_store().new_symbol(name, type);
    add_symbol(sym);
    return sym;
}

Symbol* Scope::new_symbol(Symbol* sym_old)
{
    Symbol* sym = get_symbol_store().new_symbol(sym_old);
    add_symbol(sym);
    return sym;
}

int Scope::get_ancestor_scope_that_has_symbol(string name, Scope*& pscope)
{
    pscope = m_parent_scope;
    while (!pscope->has_symbol_for_name(name)) {
        pscope = pscope->m_parent_scope;
        if (!pscope) {
            pscope = NULL;
            abort();
        }
    }
    return 1;
}

Symbol* Scope::get_symbol_for_name(string name)
{
    Scope* scope = this;
    while (!scope->has_symbol_for_name(name)) {
        try {
            scope = scope->m_parent_scope;
            if (!scope)
            return NULL;
        } catch (std::exception& e)
        {
            return NULL;
        }
    }
    Symbol* sym = scope->m_symbols.find(name)->second.back();
    
    return sym;
}

Symbol* Scope::get_return_symbol()
{
    return get_symbol_for_name("__RETURN__");
}

/**
 * Functions exposed to lexer
 */

void push_scope()
{
    Scope* new_scope = new Scope;
    get_scope_stack().push(new_scope);
}

ScopeStack& get_scope_stack()
{
    return ScopeStack::instance();
}

void pop_scope()
{
    get_scope_stack().pop();
}

Scope* get_current_scope()
{
    return get_scope_stack().top();
}

SymbolStore& get_symbol_store()
{
    return SymbolStore::instance();
}

void set_pending_symbol_type(SymbolType type)
{
    get_symbol_store().m_pending_symbol_type = type;
}

SymbolType get_pending_symbol_type()
{
    SymbolType type = get_symbol_store().m_pending_symbol_type;
    get_symbol_store().m_pending_symbol_type = NONE;
    return type;
}

Symbol* lookup(char* name, SymbolType type)
{
    Scope* top_scope = get_scope_stack().top();
    Symbol* sym = top_scope->get_symbol_for_name(name);
    if (!sym) {
        if (type == NONE) {
            FATAL("Cannot find symbol for name: " << name << ", but also cannot\
                  create new symbol of type NONE.");
        }
        
        sym = top_scope->new_symbol(name, type);
    } else {
        if (type != NONE) {
            FATAL("Found symbol for name: " << name << ", but type is not NONE , I\
                  guess you are trying to override other local variables, but it's not\
                  supported yet.");
        }
        
        sym = top_scope->get_symbol_for_name(name);
    }
    return sym;
    }
