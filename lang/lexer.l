%option noyywrap nodefault yylineno

%{

  #include "gash_lang.h"

  #include "parser.tab.h"

  extern "C" {int yylex(void);}

  int yycolumn = 1;

  // Helper macro for locating line number when error happens.
#define YY_LOC                                      \
  yylloc.first_line = yylineno;                     \
  yylloc.first_column = yycolumn;                   \
  yycolumn=yycolumn+yyleng;                         \
  yylloc.last_column = yycolumn;                    \
  yylloc.last_line = yylineno

  using gashlang::ScopeType;
  using gashlang::SymbolType;
  using gashlang::SymbolType::NUM;
  using gashlang::SymbolType::FUNC;
  using gashlang::RoleType;
  using gashlang::RoleType::rGARBLER;
  using gashlang::RoleType::rEVALUATOR;
  using gashlang::i32;
  using gashlang::i64;
  using gashlang::u32;
  using gashlang::u64;
  using gashlang::yyerror;

  using gashlang::lookup;
  using gashlang::push_scope;
  using gashlang::get_current_scope;
  using gashlang::pop_scope;
  using gashlang::set_pending_symbol_type;
  using gashlang::get_pending_symbol_type;

%}

%%

 /* Integer parser
    We only accept integer 64 here */

[-]?[0-9]+ { YY_LOC; yylval.yy_val_i64 = atoll(yytext); return I64;   }

 /* Single character operations */

"+" |
"-" |
"*" |
"/" |
"=" |
"|" |
"^" |
"&" |
"~" |
"," |
"." |
";" |
":" |
"#" |
"@" |
"[" |
"]" |
"(" |
")" |
">" |
"<" { YY_LOC; return yytext[0]; }


 /* Scope related operation */

"{" {
    YY_LOC;
    // Create a new scope and push to scope stack
    // Assign the new scope to the scope symbol
    push_scope();
    yylval.yy_scope = get_current_scope();
    return S_START;
}

"}" {
    YY_LOC;
    pop_scope();
    return S_END;
}

 /* Comparison operations */
"=="     { YY_LOC;  yylval.yy_cmp = COP_EQ; return CMP; }
">="     { YY_LOC;  yylval.yy_cmp = COP_LAE; return CMP; }
"<="     { YY_LOC;  yylval.yy_cmp = COP_LEE; return CMP; }
"!="     { YY_LOC;  yylval.yy_cmp = COP_NEQ; return CMP; }

 /* Keywords */

"if"    { YY_LOC;  return IF;    }
"then"  { YY_LOC;  return THEN;  }
"else"  { YY_LOC;  return ELSE;  }
"func"  { YY_LOC;
         // The next name will be a function name
         set_pending_symbol_type(FUNC);
         return FUNCTION;}

"return" { YY_LOC; return RETURN;}
"for"    { YY_LOC;  return FOR; }

 /* Integer Type */
"int"[1-9][0-9]*    { YY_LOC;
                      yylval.yy_intlen = atoi(yytext+3);
                      GASSERT(yylval.yy_intlen <= 64);
                      // The next name will correspond to a number
                      set_pending_symbol_type(NUM);
                      return T_INT;}

 /* Directives */

"defip"     { YY_LOC; return DEF_IP;    }
"definput"  { YY_LOC; return DEF_INPUT; }
"defrole"   { YY_LOC; return DEF_ROLE;  }
"defport"   { YY_LOC; return DEF_PORT;  }
"GARBLER"   { YY_LOC; yylval.yy_role = rGARBLER; return GARBLER;     }
"EVALUATOR" { YY_LOC; yylval.yy_role = rEVALUATOR; return EVALUATOR; }

 /* [0-9]{1,3}"."[0-9]{1,3}"."[0-9]{1,3}"."[0-9]{1,3}  { YY_LOC; */
 /*                                                      yylval.yy_ip = yytext; */
 /*                                                      return IP;} */


 /* Comment */

"//".*

 /* Ignore white spaces */

[ \t]

 /* User defined names */

[a-zA-Z_][a-zA-Z0-9_]*    { YY_LOC;
    yylval.yy_sym = lookup(yytext, get_pending_symbol_type());
    return NAME; }

 /* Any other symbols */

.       { YY_LOC; yyerror("Mystery character %c\n", *yytext); }

%%
