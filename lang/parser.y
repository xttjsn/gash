/* Parser of gashlang */

%{
  #include <stdio.h>
  #include <stdlib.h>
  #include "gash_lang.h"

  extern "C" {int yyparse(void); int yylex(void); int yywrap() { return 1;} }
  extern "C" {void yyerror(const char *s, ...); }

  using gashlang::Ast;
  using gashlang::Symbol;
  using gashlang::Vardef;
  using gashlang::Scope;
  using gashlang::RoleType;

  using gashlang::new_if;
  using gashlang::new_for;
  using gashlang::new_ast;
  using gashlang::new_num;
  using gashlang::new_dir;
  using gashlang::new_ret;
  using gashlang::get_current_scope;
%}

/* Enable tracing. Disable it in release version. */
%debug

 /* Set debug level to verbose. */
%error-verbose

 /* Keep track of current location during parsing. */
%locations

 /* Union of yylval */
%union {
  i32 yy_val_i32;
  i64 yy_val_i64;
  u32 yy_val_u32;
  u64 yy_val_u64;
  u32 yy_cmp;
  u32 yy_intlen;
  Ast* yy_ast;
  Symbol* yy_sym;
  Vardef* yy_vardef;
  Vardeflist* yy_vardeflist;
  Scope* yy_scope;
  Tuple* yy_arrtuple;
  char* yy_ip;
}

 /*** Declare tokens ***/

 /* Basic numeric types */
%token <yy_val_i32> I32
%token <yy_val_i64> I64
%token <yy_val_u32> U32
%token <yy_val_u64> U64

 /* User defined name/symbol */
%token <yy_sym> NAME

 /* End of file */
%token ENDOFFILE EOL

 /* Function and flow control */
%token FUNC
%token IF
%token THEN
%token ELSE
%token FOR
%token RETURN

 /* Directives */
%token DEF_INPUT
%token DEF_ROLE
%token DEF_PORT
%token DEF_IP
%token DEF_START
%token GARBLER
%token EVALUATOR

 /* Non-associative operator */
%nonassoc <yy_cmp> CMP
%nonassoc <yy_scope> S_START
%nonassoc S_END
%nonassoc UMINUS UNEG

 /* Associative operators */
%right '='
%left '+' '-' '|' '&' '^' '.'
%left '*' '/'

 /* Grammar elements */
%type <yy_ast> exp
%type <yy_ast> stmt
%type <yy_ast> stmtlist
%type <yy_ast> ret
%type <yy_ast> dir
%type <yy_ast> dirlist
%type <yy_ast> numlist
%type <yy_vardef> vardef
%type <yy_vardeflist> vardeflist
%start program

%%

stmt: IF exp S_START stmtlist S_END {
  // create a if statement with the original scope and the inner scope.
  $$ = newif($2, $3, $4, get_current_scope());
 }
| IF exp S_START stmtlist S_END ELSE S_START stmtlist S_END {
  $$ = newifelse($2, $3, $4, $7, $8, get_current_scope());
 }
| FOR '(' exp ';' exp ';' exp ')' S_START stmtlist S_END {
  $$ = newfor($3, $5, $7, $9, $10, get_current_scope());
 }
| exp ';'                             // Nothing, default behavior is assigning $1 to $$
| mytypedef ';'                       { $$=NULL;                               }
| ret ';'                             // Default behavior
;


stmtlist: stmt      					        { $$ = new_ast(nSTMTLIST, $1, NULL);	   }
| stmt  list                          { $$ = new_ast(nSTMTLIST, $1, $2);       }
;

numlist : NUMBER_I64                  { $$ = new_numlist($1, NULL);            }
| NUMBER_I64 ',' numlist              { $$ = new_numlist($1, $3);              }
;


dir : '#' DEF_INPUT NAME I64          { $$ = dir_input($3, $4);                }
| '#' DEF_ROLE GARBLER                { $$ = dir_role(GARBLER);                }
| '#' DEF_ROLE EVALUATOR              { $$ = dir_role(EVALUATOR);              }
| '#' DEF_PORT I64                    { $$ = dir_port($3);                     }
| '#' DEF_IP IP                       { $$ = dir_ip($3);                       }
;


dirlist: dir                          { $$ = $1;                              }
| dir dirlist                         { $$ = new_ast(nDIRLIST, $1, $2);       }
;

exp: exp CMP exp                      { $$ = new_cmp($2, $1, $3);             }
| exp '+' exp			                    { $$ = new_ast(nPLUS, $1, $3);			    }
| exp '-' exp				                  { $$ = new_ast(nMINUS, $1, $3);		      }
| exp '*' exp				                  { $$ = new_ast(nMUL, $1, $3);			      }
| exp '/' exp				                  { $$ = new_ast(nDIV, $1, $3);			      }
| exp '|' exp                         { $$ = new_ast(nBOR, $1, $3);           }
| exp '&' exp                         { $$ = new_ast(nBAND, $1, $3);          }
| exp '^' exp                         { $$ = new_ast(nBXOR, $1, $3);          }
| '~' exp %prec UNEG                  { $$ = new_ast(nBNEG, $2);              }
| '(' exp ')'					                { $$ = $2;							                }
| I64						                      { $$ = new_num($1);					            }
| '-' exp %prec UMINUS			          { $$ = new_ast(nUMINUS, $2, NULL);	    }
| NAME							                  { $$ = new_ref(rNORMAL, $1);	          }
| exp '=' exp                         { $$ = new_asgn($1, $3);                }
| vardef '=' exp                      { $$ = new_initasgn($1, $3);            }
| NAME '.' exp                        { $$ = new_ref(rBIT, $1, $3);           }
;

ret: RETURN exp                 		  { $$ = new_return($2);                  }
;

vardef: T_INT NAME                    { $$ = new_vardef($1, $2);              }
;

vardeflist: vardef                    { $$ = new_vardeflist($1, NULL);        }
| vardef ',' vardeflist               { $$ = new_vardeflist($1, $2);          }

program: dirlist FUNC NAME '(' vardeflist ')' SCOPE_START list SCOPE_END ENDOFFILE {

  // Define function, which will puts everything under the func struct named NAME
  // However, current we don't support function call, so it's useless
  defun($3, $5, $8);

  // Recursively executing the `list`
  output($8);

  // Write down the built-up circuit to file
  writeCircuit();

  // Evaluate the directives, which puts related field to a ExeCtx class, which is used by the gc framework
  evaldir($1);

  // Write the data defined by the directives to file
  writeData();

  // Remove all dynamically allocated memory
  cleanup($8);

  // Parse finished
  return 0;
 }
