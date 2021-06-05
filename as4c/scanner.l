/*
 * Lexer definition for simplified Python syntax.
 */

/*
 * Since we're only parsing 1 file, we don't need to have yywrap() (plus,
 * having it included messes up compilation).
 */
%option noyywrap

%option yylineno

%{
#include <iostream>
#include <stack>
#include <cstdlib>

#include "parser.hpp"

YYSTYPE yylval;
YYLTYPE yylloc;
yypstate* pstate = yypstate_new();

using namespace std;

#define PUSH_TOKEN(category) do {                      \
    int s = yypush_parse(pstate, category, &yylval, &yylloc);   \
    if (s != YYPUSH_MORE) {                         \
      yypstate_delete(pstate);                      \
      return s;                                     \
    }                                               \
  } while(0)

#define YY_USER_ACTION \
  yylloc.first_line = yylloc.last_line = yylineno;

/*
 * We'll use this stack to keep track of indentation level, as described in
 * the Python docs:
 *
 * https://docs.python.org/3/reference/lexical_analysis.html#indentation
 */
std::stack<int> _indent_stack;
%}

%%

%{
  /*
   * These lines go at the top of the lexing function.  We only want to
   * initialize the indentation level stack once by pushing a 0 onto it (the
   * indentation stack should never be empty, except immediately after it is
   * created).
   */
  if (_indent_stack.empty()) {
    _indent_stack.push(0);
  }
%}

^[ \t]*\r?\n   { /* Skip blank lines */  }
^[ \t]*#.*\r?\n    { /* Skip whole-line comments. */  }
#.*$          { /* Skip comments on the same line as a statement. */ }

^[ \t]+       {

                if (_indent_stack.top() < yyleng) {
                  yylval.category = INDENT; PUSH_TOKEN(yylval.category);
                  _indent_stack.push(yyleng);

                } else {
                  while (!_indent_stack.empty() && _indent_stack.top() != yyleng) {
                    _indent_stack.pop();
                    yylval.category = DEDENT; PUSH_TOKEN(yylval.category);
                  }
                  if (_indent_stack.empty()) {
                    std::cerr << "Error: Incorrect indentation on line " << yylineno << std::endl;
                    return 1;
                  }
                }
              }

^[^ \t\r\n]+  {
                while (_indent_stack.top() != 0) {
                  _indent_stack.pop();
                  yylval.category = DEDENT; PUSH_TOKEN(yylval.category);
                }
                REJECT;
              }

\r?\n         {
                yylval.category = NEWLINE; PUSH_TOKEN(yylval.category);
              }

<<EOF>>       {
                /*
                 * If we reach the end of the file, pop all indentation levels
                 * off the stack, and emit a DEDENT for each one.
                 */
                while(_indent_stack.top() != 0) {
                  _indent_stack.pop();
                  PUSH_TOKEN((yylval.category=DEDENT));
                }
               int s = yypush_parse(pstate, 0, NULL, NULL);
              yypstate_delete(pstate);
              return s;
              }

[ \t]         { /* Ignore spaces that haven't been handled above. */ }

"and"         { yylval.category = AND; PUSH_TOKEN(yylval.category);}
"break"       { yylval.category = BREAK; PUSH_TOKEN(yylval.category);}
"def"         { yylval.category = DEF; PUSH_TOKEN(yylval.category);}
"elif"        { yylval.category = ELIF; PUSH_TOKEN(yylval.category);}
"else"        { yylval.category = ELSE; PUSH_TOKEN(yylval.category);}
"for"         { yylval.category = FOR;PUSH_TOKEN(yylval.category);}
"if"          { yylval.category = IF; PUSH_TOKEN(yylval.category);}
"not"         { yylval.category = NOT; PUSH_TOKEN(yylval.category);}
"or"          { yylval.category = OR; PUSH_TOKEN(yylval.category);}
"return"      { yylval.category = RETURN; PUSH_TOKEN(yylval.category);}
"while"       { yylval.category = WHILE; PUSH_TOKEN(yylval.category);}
"True"        { yylval.str = new string(yytext,yyleng); PUSH_TOKEN(TRUE);}
"False"       { yylval.str = new string(yytext,yyleng);PUSH_TOKEN(FALSE);}

[a-zA-Z_][a-zA-Z0-9_]*  {
                          yylval.str = new string(yytext,yyleng);
                          PUSH_TOKEN(IDENTIFIER);
                        }

-?[0-9]*"."[0-9]+       {
                          yylval.str = new string(yytext,yyleng);
                          PUSH_TOKEN(FLOAT);
                        }

-?[0-9]+      {
                yylval.str = new string(yytext,yyleng);
                PUSH_TOKEN(NUMBER);
              }

"="           { yylval.category = EQUALS; PUSH_TOKEN(yylval.category);}
"+"           { yylval.category = PLUS;PUSH_TOKEN(yylval.category);}
"-"           { yylval.category = MINUS; PUSH_TOKEN(yylval.category);}
"*"           { yylval.category = TIMES; PUSH_TOKEN(yylval.category);}
"/"           { yylval.category = DIVIDEDBY; PUSH_TOKEN(yylval.category);}
"=="          { yylval.category = EQ; PUSH_TOKEN(yylval.category);}
"!="          {  yylval.category = NEQ; PUSH_TOKEN(yylval.category);}
">"           { yylval.category = GT; PUSH_TOKEN(yylval.category);}
">="          { yylval.category = GTE; PUSH_TOKEN(yylval.category);}
"<"           { yylval.category = LT; PUSH_TOKEN(yylval.category);}
"<="          { yylval.category = LTE; PUSH_TOKEN(yylval.category);}
"("           { yylval.category = LPAREN; PUSH_TOKEN(yylval.category);}
")"           { yylval.category = RPAREN; PUSH_TOKEN(yylval.category);}
","           { yylval.category = COMMA; PUSH_TOKEN(yylval.category);}
":"           { yylval.category = COLON; PUSH_TOKEN(yylval.category);}

.             {
                std::cerr << "Unrecognized token on line " << yylineno << ": "
                  << yytext << std::endl;
                return 1;
              }

%%
