//
//  lang.hpp
//  gash
//
//  Created by Xiaoting Tang on 6/18/18.
//  Copyright © 2018 Xiaoting Tang. All rights reserved.
//

#ifndef lang_hpp
#define lang_hpp

#include <stdio.h>

#include <boost/program_options.hpp>
#include "common.hpp"
#include "garbler.hpp"
#include "evaluator.hpp"
#include "circuit.hpp"
#include "sym.hpp"

namespace po = boost::program_options;

/* External: interface to the lexer */
extern int yylineno;    /* from lexer */
extern "C" int yyparse(void);

/* External: interface to parser */
struct YYLTYPE;
extern "C" YYLTYPE yylloc;

extern int numIN;
extern int numOUT;
extern int numAND;
extern int numOR;
extern int numXOR;
extern int numDFF;

    
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
 * Symbol related classes declarations
 */
class Scope;
class Symbol;
class NumSymbol;
class FuncSymbol;

/**
 * A enumeration type for Abstract Syntax Tree
 * Each type correspond to a class
 * TODO: consider remove the nDIR type
 */
typedef enum {
    /* List */
    nLIST,
    
    /* Arithmetic operations */
    nAOP,
    
    /* Bitwise operations */
    nBOP,   // AND, OR, XOR, or Invert, left shift or right shift
    
    /* Comparison/logical/boolean operations */
    nCOP,
    
    /* Language structure types */
    nNAME,  // Name
    nREF,   // Reference
    nASGN,  // Assign
    nNUM,   // Number
    nRET,   // Return
    nVDF,   // Variable definition
    
    /* Flow structure */
    nIF,
    nIFEL,  // If else
    nFOR,
    
    /* Function */
    nFUNC,
    
    /* Directive */
    nDIR,
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

typedef enum {
    rGARBLER,
    rEVALUATOR,
} RoleType;

/**
 * Abstract syntax tree definition
 * The trick is that every language structure has m_nodetype as its first
 * member, therefore we can safely cast an Ast to the appropriate class
 */
class Ast {
    public:
    NodeType m_nodetype = nLIST;
    Ast *m_left;
    Ast *m_right;
};

/**
 * Arithmetic operations
 *
 */
class Aop {
    public:
    NodeType m_nodetype = nAOP;
    uint32_t m_op;
    Ast* m_left;
    Ast* m_right;
};

/**
 * Bit-wise operations
 *
 * @return
 */
class Bop {
    public:
    NodeType m_nodetype = nBOP;
    /// Used for SHL/SHR
    uint32_t m_op;
    Ast* m_left;
    Ast* m_right;
    Ast* m_n_ast;
};

/**
 * Comparison/logical/boolean operation;
 *
 */
class Cop {
    public:
    NodeType m_nodetype = nCOP;
    uint32_t m_op;
    Ast* m_left;
    Ast* m_right;
};

/**
 * If flow class
 *
 */
class If {
    public:
    NodeType m_nodetype = nIF;
    Ast* m_cond_ast;
    Ast* m_if_ast;
    Scope* m_if_scope;
    Scope* m_prev_scope;
};

/**
 * If else flow class
 *
 */
class Ifel {
    public:
    NodeType m_nodetype = nIFEL;
    Ast* m_cond_ast;  // The ast for the condition
    Ast* m_if_ast;  // The ast of code that's executed if condition is true
    Ast* m_else_ast;  // The ast of code that's executed if condition is false
    Scope* m_if_scope;
    Scope* m_else_scope;
    Scope* m_prev_scope;
};

/**
 * For flow class
 * Note that `init`, `inc`, `cond` is not executed using circuit evaluation but using normal evaluation
 */
class For {
    public:
    NodeType m_nodetype = nFOR;
    Ast* m_init_ast;  // The ast for initialization, it only gets to run once
    Ast* m_inc_ast;   // The ast for increment, it gets to run if cond is true
    Ast* m_cond_ast;  // The ast for condition, it gets to run once `do` finishes
    Ast* m_do_ast;    // The ast for doing the actual job, it's evaluated using circuit evaluation
    Scope* m_for_scope;
    Scope* m_prev_scope;
};

/**
 * Num class
 *
 */
class Num {
    public:
    NodeType m_nodetype = nNUM;
    int64_t m_val;
};

/**
 * Reference parent class
 *
 */
class Ref {
    public:
    NodeType m_nodetype = nREF;
    RefType m_reftype;
    Symbol* m_sym;
    /// The scope at the time of parsing
    Scope* m_scope;
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
    Ast* m_bit_idx_ast;
};

/**
 * An assignment operation
 *
 */
class Asgn {
    public:
    NodeType m_nodetype = nASGN;
    Ast* m_left;          // m_left can be a ref or a vardef
    Ast* m_right;         // m_right can be a number, or a ref
};

/**
 * Language return
 *
 */
class Ret {
    public:
    NodeType m_nodetype = nRET;
    Ast* m_ret;          // m_ret can be a number, a ref or some expression
    Func* m_func;        // The function
};

/**
 * Variable definition (single int64_t value only)
 *
 */
class Vardef {
    public:
    NodeType   m_nodetype = nVDF;
    Symbol*    m_sym;
    bool       m_isinput;
};

/**
 * Function definition
 *
 */
class Func {
    public:
    NodeType m_nodetype = nFUNC;
    FuncSymbol* m_fsym;
    Ast* m_vdf_ast;
    Ast* m_do_ast;
};

/**
 * Directive
 * NOTE: only used for input directive
 */
class Dir {
    public:
    NodeType m_nodetype = nDIR;
    
    /// The symbol that this directive is going to provide input. Could be NULL
    /// if the directive does not use it
    Symbol* m_sym;
    
    /// The value that this directive provides, used with m_sym
    int64_t m_val;
};

/**
 * Define a function
 *
 * @param sym
 * @param vdf_ast
 * @param do_ast
 *
 * @return
 */
Func* defun(Symbol* sym, Ast* vdf_ast, Ast* do_ast);


/**
 * Create a new Ast from left Ast and right Ast
 *
 * @param nodetype
 * @param left
 * @param right
 *
 * @return
 */
Ast* new_ast(Ast* left, Ast* right);

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
Ast* new_ref_int(Symbol* sym, Scope* scope);

/**
 * Create new bit reference
 *
 * @param sym
 * @param bit_idx_ast
 *
 * @return
 */
Ast* new_ref_bit(Symbol* sym, Ast* bit_idx_ast, Scope* scope);

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
Ast* new_num(int64_t val);

/**
 * Create an if flow
 *
 * @param cond
 * @param if_ast
 * @param if_scope
 * @param prev_scope
 *
 * @return
 */
Ast* new_if(Ast* cond, Ast* if_ast, Scope* if_scope, Scope* prev_scope);

/**
 * Create an if else flow
 *
 * @param cond
 * @param if_ast
 * @param else_ast
 * @param if_scope
 * @param else_scope
 * @param prev_scope
 *
 * @return
 */
Ast* new_ifelse(Ast* cond, Ast* if_ast, Ast* else_ast, Scope* if_scope, Scope* else_scope, Scope* prev_scope);

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
 *
 * @return
 */
Ast* new_vdf(Symbol* sym, uint32_t intlen);

/**
 * Directive on input
 *
 * @param sym
 * @param val
 */
Ast* new_dir_input(Symbol* sym, int64_t val);

/**
 * Directive on ip
 *
 * @param ip
 */
void dir_ip(const char* ip);

void set_garbler(Garbler* g);

void set_evaluator(Evaluator* e);

/**
 * The core function of circuit building.
 * It takes a statement and evaluate it one after another (from right to left)
 * recursively.
 *
 * @param ast The ast that we want to evaluate
 * @param bundle The return bundle, allocate it on the stack please
 */
void evalast(Ast* ast, Bundle& bundle);

/**
 * This is another evaluation function. However, this function has no side
 * effect on circuit building, its only (now) purpose is to serve for loop's
 * init, condition, and increment
 *
 * @param ast The ast we want to evaluate
 *
 * @return
 */
int64_t evalast_n(Ast* ast);

/**
 * Error printing
 *
 * @param s
 */
void yyerror(const char *s, ...);

void cleanup();

/**
 * Clean all intermediate parsing related structures.
 * Possibly for the purpose of conducting unit test.
 * Will destroy all scopes in the scope stack, and
 * re-initialize all global variables.
 *
 */
void parse_clean();

#endif /* lang_hpp */
