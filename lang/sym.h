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

#include "common.h"
#include "circuit.h"

namespace gashlang {

  class Symbol;
  class SymbolStore;
  class Scope;
  class ScopeStack;


  /**
   * The symbol classes
   *
   */
  enum SymbolType {
    NUM,
    ARRAY,
    FUNC
  };

  class Symbol {
  public:
    string m_name;
    u64 m_value = -1;
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
    NumSymbol(string name, u32 version);
    NumSymbol(NumSymbol& rhs);
  };

  /**
   * Symbol that represents an array.
   */
  class ArraySymbol : public Symbol {
  public:
    vector<Bundle> m_bundles;
    ArraySymbol(string name, u32 version);
    ArraySymbol(ArraySymbol& rhs);
  };

  /**
   * Symbol that represents a function.
   */
  class FuncSymbol : public Symbol {
  public:
    ast* m_func;
    FuncSymbol(string name, u32 version);
    FuncSymbol(FuncSymbol& rhs);
  };

  typedef vector<string, vector<Symbol*> > NameSymbolsMap;

  class SymbolStore {
    /// Store all the symbols in a single compilation
    NameSymbolsMap m_symbols;
  public:

    SymbolStore() {}

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
    Symbol* new_symbol(Symbol* old_sym);

  };

  enum ScopeType {
    FUNCTION,
    BLOCK
  };

  class Scope {
    NameSymbolsMap m_symbols;
  public:
    Scope* m_parent_scope;
    ScopeType m_type;

    Scope() {}

    bool has_symbol_for_name(string name);

    void add_symbol(Symbol* sym);

    Symbol* new_symbol(string name);

    Symbol* new_symbol(Symbol* sym_old);

    Symbol* get_symbol_for_name(string name);

    Symbol* get_return_symbol();

    u32 get_newest_version_for_name(string name);

    Symbol* get_symbol_for_id_and_version(string name, u32 version);
  };

  /**
   * Store all the scopes along compilation.
   * The reason to use another wrapper class around a std::stack is to enable singleton design.
   */
  class ScopeStack {
    std::stack<Scope*> m_scopes;

    ScopeStack() {}

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
    }
  };

#define get_scope_stack()                       \
  ScopeStack::instance()

#define push_scope(scope)                       \
  get_scope_stack().push(scope)

#define pop_scope()                             \
  get_scope_stack().pop()

#define current_scope()                         \
  get_scope_stack().top()

  /**
   * Function: symbol lookup from the most recent scope.
   */
  Symbol* lookup(char* name);
  Symbol* lookup(string name);

  /**
   * Look up symbol with name 'name' of version 'version' in scope store (i.e. global scope).
   *
   * @param name
   * @param version
   *
   * @return
   */
  Symbol* lookup(string name, u32 version);

  /**
   * Look up symbol with 'name' of version 'version' in scope 'scope'.
   *
   * @param name
   * @param version
   * @param scope
   *
   * @return
   */
  Symbol* lookup(string name, u32 version, Scope* scope);

}  // gashlang

#endif
