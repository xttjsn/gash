%option noyywrap nodefault yylineno

%{

  #include "gash_lang.h"

  #include "gash_lang.tab.h"

  extern "C" {int yylex(void);}

  int yycolumn = 1;

  // Helper macro for locating line number when error happens.
#define YY_LOC
  yylloc.first_line = yylineno;                     \
                                                    \
  yylloc.first_column = yycolumn;                   \
                                                    \
  yycolumn=yycolumn+yyleng;                         \
                                                    \
  yylloc.last_column = yycolumn;                    \
                                                    \
  yylloc.last_line = yylineno

  using gashlang::push_scope;

  using gashlang::get_current_scope;

  using gashlang::pop_scope;

  using gashlang::set_pending_scope_type;

  using gashlang::get_pending_scope_type;

  using gashlang::ScopeType;
%}

%%

 /* Integer parser */

[-]?[0-9]+ { YY_LOC; yylval.ld = atoll(yytext); return NUMBER_I64;   }


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
"_" { YY_LOC; return yytext[0]; }


 /* Scope related operation */

"{" {
    YY_LOC;
    push_scope();
    yylval.ss = get_current_scope();
    return SCOPE_START;
}
"}" {
    YY_LOC;
    pop_scope();
    return SCOPE_END;
}

 /* Binary comparison related operations */

">"      { YY_LOC;  yylval.fn = nLA; return CMP; }
"<"      { YY_LOC;  yylval.fn = nLE; return CMP; }
"<>"     { YY_LOC;  yylval.fn = nNEQ; return CMP; }
"=="     { YY_LOC;  yylval.fn = nEQ; return CMP; }
">="     { YY_LOC;  yylval.fn = nLAEQ; return CMP; }
"<="     { YY_LOC;  yylval.fn = nLEEQ; return CMP; }
"!="     { YY_LOC;  yylval.fn = nEQ; return CMP; }

 /* Keywords */

"if"    { YY_LOC;  return IF;    }
"then"  { YY_LOC;  return THEN;  }
"else"  { YY_LOC;  return ELSE;  }
"func"  { YY_LOC;
         set_next_scope_type(ScopeType::FUNC);
         return FUNC;}

"return" { YY_LOC; return RETURN;}
"for"    { YY_LOC;  return FOR; }
"int"[1-9][0-9]*    { YY_LOC;
                      yylval.intlen = atoi(yytext+3);
                      static_assert(yylval.intlen <= 64, "Int length larger than 64.");
                      return INT;}
"bundle" { YY_LOC; return BUNDLE;}

 /* Directives */

"defip"     { YY_LOC; return DEF_IP;    }
"definput"  { YY_LOC; return DEF_INPUT; }
"defrole"   { YY_LOC; return DEF_ROLE;  }
"defport"   { YY_LOC; return DEF_PORT;  }
"defstart"  { YY_LOC; return DEF_START; }
"defexpout" { YY_LOC; return DEF_EXPOUT;}
"GARBLER"   { YY_LOC; return GARBLER;   }
"EVALUATOR" { YY_LOC; return EVALUATOR; }

 /* Comment */

"//".*

 /* Ignore white spaces */

[ \t]

/* Ignore line continuation */

\\\n

 /* Ignore newline */

\n

 /* End of file */

<<EOF>>	{ YY_LOC; return ENDOFFILE;		}

 /*
  * User defined names.
  */
[a-zA-Z_][a-zA-Z0-9_]*    { YY_LOC;
                            yylval.s = lookup(yytext, get_next_scope_type());
                            return NAME; }
