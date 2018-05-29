/*
 * sym.h -- Symbol related definition
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

#ifndef GASH_SYM_H
#define GASH_SYM_H

#include "../include/common.hh"
#include "circuit.hh"
#include "gash_lang.hh"

namespace gashlang {

  class Symbol;
  class SymbolStore;
  class Scope;
  class ScopeStack;
  class Ast;


  /**
   * The symbol classes
   *
   */
  enum SymbolType {
    NUM,
    ARRAY,
    FUNC,
    NONE // reserved for lexer when there's no pending scope type and it wants
         // to call look up
  };

  class Symbol {
  public:
    string m_name;
    i64 m_value = -1;
    u32 m_version;
    SymbolType m_type;

    Symbol(string name, u32 version) : m_name(name), m_version(version) {}
    inline void gets_older () {
      m_version++;
    }
  };

  /**
   * Symbol that represents a single number.
   */
  class NumSymbol : public Symbol {
  public:
    Bundle m_bundle;
    u32 m_len;
    NumSymbol(string name, u32 version);
    NumSymbol(NumSymbol& rhs);
  };

  /**
   * Symbol that represents an array.
   */
  class ArraySymbol : public Symbol {
  public:
    vector<Bundle> m_bundles;
    u32 m_len;
    u32 m_arrlen;
    ArraySymbol(string name, u32 version);
    ArraySymbol(ArraySymbol& rhs);
    void get_bundle(Bundle& bret);
  };

  /**
   * Symbol that represents a function.
   */
  class FuncSymbol : public Symbol {
  public:
    Ast* m_func;
    FuncSymbol(string name, u32 version);
    FuncSymbol(FuncSymbol& rhs);
  };

  typedef map<string, vector<Symbol*> > NameSymbolsMap;

  class SymbolStore {
    /// Store all the symbols in a single compilation
    NameSymbolsMap m_symbols;
    SymbolStore() {}
  public:

    SymbolType m_pending_symbol_type;

    /**
     * Make SymbolStore Singleton
     *
     * @return
     */
    static SymbolStore& instance() {
      static SymbolStore instance;
      return instance;
    }

    SymbolStore(SymbolStore const&)     = delete;
    void operator=(SymbolStore const&)  = delete;

    /**
     * Check whether there's a symbol associated with that name
     *
     * @param name
     *
     * @return
     */
    bool has_symbol_for_name(string name);

    /**
     * Check whether there's a symbol associated with that name, if not, abort.
     *
     * @param name
     */
    void require_has_symbol_for_name(string name);

    /**
     * Check whether all versions for all symbols are contiguously increasing.
     *
     * @return
     */
    bool version_consistency_check();

    /**
     * Do the version consistency check only for the one that has the name.
     *
     * @param name
     *
     * @return
     */
    bool version_consistency_check_for_name(string name);

    /**
     * Return the highest version of symbol associated with the name.
     *
     * @param name
     *
     * @return
     */
    u32 get_newest_version_for_name(string name);

    /**
     * Return the symbol associated with the name of newest version.
     *
     * @param name
     *
     * @return
     */
    Symbol* get_symbol_for_name(string name);

    /**
     * Create a new symbol with that name.
     *
     * @param name
     *
     * @return
     */
    Symbol* new_symbol(string name, SymbolType type);

    /**
     * Create a new symbol with the same name as the old symbol.
     *
     * @param old_sym
     *
     * @return
     */
    Symbol* new_symbol(Symbol* sym_old);

    /**
     * Remove all symbols
     *
     */
    void clear();

  };

  enum ScopeType {
    FUNCTION,
    BLOCK,
  };

  class Scope {
  public:
    NameSymbolsMap m_symbols;
    Scope* m_parent_scope;
    ScopeType m_type;

    Scope() {}

    bool has_symbol_for_name(string name);

    void add_symbol(Symbol* sym);

    Symbol* new_symbol(string name, SymbolType type);

    Symbol* new_symbol(Symbol* sym_old);

    /**
     * Get an ancestor scope that has a symbol with the name `name`.
     *
     * @param name
     * @param pscope
     *
     * @return
     */
    int get_ancestor_scope_that_has_symbol(string name, Scope*& pscope);

    /**
     * Get a symbol by searching from this scope downwards.
     * Will return NULL if nothing is found.
     *
     * @param name
     *
     * @return
     */
    Symbol* get_symbol_for_name(string name);

    Symbol* get_return_symbol();
  };

  /**
   * Store all the scopes along compilation.
   * The reason to use another wrapper class around a std::stack is to enable singleton design.
   */
  class ScopeStack {
    std::stack<Scope*> m_scopes;

    ScopeStack() {
      m_scopes.push(new Scope());
    }

  public:

    static ScopeStack& instance()
    {
      static ScopeStack    instance;
      return instance;
    }

    ScopeStack(ScopeStack const&)     = delete;

    void operator=(ScopeStack const&)  = delete;

    Scope* top() {
      return m_scopes.top();
    }

    void push(Scope* new_scope) {
      if (m_scopes.size() > 0)
        new_scope->m_parent_scope = m_scopes.top();
      m_scopes.push(new_scope);
    }

    void pop() {
      m_scopes.pop();
    }

    int size() {
      return m_scopes.size();
    }

    void clear() {
      while (m_scopes.size() != 0) {
        Scope* s = m_scopes.top();
        delete s;
        m_scopes.pop();
      }
      // The initial scope
      m_scopes.push(new Scope());
    }
  };


  /**
   * Functions exposed to lexer.
   */


  ScopeStack& get_scope_stack();

  /**
   * Create a new scope and put it on the top of the scope stack.
   *
   */
  void push_scope();

  /**
   * Pop the top most scope
   *
   */
  void pop_scope();

  /**
   * Return the top most scope
   *
   * @return
   */
  Scope* get_current_scope();

  /**
   * Get the global symbol store
   *
   * @return
   */
  SymbolStore& get_symbol_store();

  /**
   * Set the symbol type of the next symbol.
   *
   * @param type
   */
  void set_pending_symbol_type(SymbolType type);

  /**
   * Get the pending symbol type.
   *
   * @return
   */
  SymbolType get_pending_symbol_type();

  /**
   *
   * Symbol lookup from the most recent scope.
   * @param name
   * @param type
   *
   * @return
   */
  Symbol* lookup(char* name, SymbolType type);

}  // gashlang

#endif
