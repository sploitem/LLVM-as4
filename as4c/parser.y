%{
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include "parser.hpp"
#include "main.hpp"
using namespace std;
vector<string> symb;
string* lines;

struct node *nodes;
extern int yylex(); //defined elsewhere.
extern int yylineno;
void yyerror(YYLTYPE* loc, const char* err) {
  std::cerr << "Error (line " << loc->first_line << "): " << err << std::endl;
}



%}


/* %define api.value.type { std::string* } */
%union {
  std::string* str;
  struct node* exp_nodes;
  int category;
}

%locations
%define parse.error verbose

%define api.pure full
%define api.push-pull push


//%token to specify every each teminal symbols we will use
%token <str> IDENTIFIER NUMBER FLOAT TRUE FALSE
%token <category> EQUALS PLUS MINUS TIMES DIVIDEDBY NEWLINE COMMENT
%token <category> LPAREN RPAREN COMMA COLON SEMICOLON
%token <category> EQ NEQ GT GTE LT LTE
%token <category> AND BREAK DEF ELIF ELSE FOR IF
%token <category> NOT OR RETURN WHILE INDENT DEDENT

%type  <exp_nodes>  expression noterminal logical conditional elseconditional ifstatement whilestatement breakstatement
%type  <exp_nodes> program statements statement id
%left OR
%left AND
%left PLUS MINUS
%left TIMES DIVIDEDBY
%left EQ NEQ GT GTE LT LTE
%right NOT

// statement represents a single assignment statement
// program represents an entire program

%start program

%%

program: statements{$$ = new node; $$ = $1;  $$->label = "Block"; nodes = $$;nodes->id = "n0";}
     ;
statements: statement  {$$ = new node; 
                        $$->next_node.push_back($1); }
          | statements statement { $$ = new node;
                                   $1->next_node.push_back($2);
                                   $$=$1;
                                   }
     ;
statement: id EQUALS expression NEWLINE {
                                                  $$ = new node;
                                                  $$->label = "Assignment";
                                                  $$->next_node.push_back($1);
                                                  $$->next_node.push_back($3);
                                          
                                                  }
         | ifstatement  { $$ = new node; $$ = $1; }
         | whilestatement { $$ = new node;$$ = new node;$$= $1; }
         | breakstatement { $$ = new node;$$ = new node;$$ = $1; }
         ;
id:IDENTIFIER               {         $$ = new node;
                                      $$->label="Identifier: "; $$->shape = "box";
                                      string temp = *$1;
                                      $$->label = $$->label + temp;$$->str = *$1;
                                      }
breakstatement: BREAK NEWLINE {$$ = new node;$$->label = "break"; }
              ;
expression: noterminal                        { $$ = new node;$$ = $1; }
          | expression PLUS expression        { $$ = new node;$$->label = "PLUS"; $$->next_node.push_back($1);$$->next_node.push_back($3);}
          | expression TIMES expression       { $$ = new node;$$->label = "TIMES"; $$->next_node.push_back($1);$$->next_node.push_back($3);}
          | expression MINUS expression       { $$ = new node;$$->label = "MINUS"; $$->next_node.push_back($1);$$->next_node.push_back($3);}
          | expression DIVIDEDBY expression   { $$ = new node;$$->label = "DIVIDEDBY"; $$->next_node.push_back($1);$$->next_node.push_back($3);}
          ;
noterminal: IDENTIFIER               {$$ = new node;$$->label="Identifier "; $$->shape = "box"; $$->str = *$1;
                                      $$->label = $$->label + *$1;}
          | NUMBER                   {$$ = new node;$$->label="Integer "; $$->shape = "box"; $$->str = *$1;                               
                                      $$->label = $$->label + *$1; }
          | FLOAT                    {$$ = new node;$$->label="Float "; $$->shape = "box"; $$->str = *$1;
                                      $$->label = $$->label + *$1; }
          | TRUE                     {$$ = new node;$$->label="boolean true"; $$->shape = "box"; $$->str = *$1; }
          | FALSE                    {$$ = new node;$$->label="boolean false"; $$->shape = "box";  $$->str = *$1;}
          | LPAREN expression RPAREN {$$ = new node;$$ = $2;}
          ;
ifstatement:IF logical COLON NEWLINE conditional elseconditional {
                          $$ = new node;
                          $$->next_node.push_back($2);
                          $$->label = "If";
                          $$->next_node.push_back($5);
                          if($6->label !="NULL"){
                               $$->next_node.push_back($6);
                          }
                         }
           ;
whilestatement:WHILE logical COLON NEWLINE conditional DEDENT {
                                                                $$ = new node;
                                                                $$->next_node.push_back($2);
                                                                $$->label = "While";
                                                                $$->next_node.push_back($5); }
              ;
conditional: INDENT statements {$$ = new node;$$=$2; $$->label="Block"; }
           ;
elseconditional: DEDENT {$$= new node;  $$->label = "NULL";}
               | DEDENT ELSE COLON NEWLINE conditional DEDENT {$$ = new node;$$ = $5;$$->str="else"; }
               ;
logical:  noterminal             { $$ = new node;$$ = $1; }
          | NOT logical          { $$ = new node;$$->label="NOT";$$ = $2; }
          | logical LT logical   { $$ = new node;$$->label="LT"; $$->next_node.push_back($1);$$->next_node.push_back($3); }
          | logical LTE logical  { $$ = new node;$$->label="LTE"; $$->next_node.push_back($1);$$->next_node.push_back($3);}
          | logical AND logical  { $$ = new node;$$->label="AND"; $$->next_node.push_back($1);$$->next_node.push_back($3); }
          | logical GT logical   { $$ = new node;$$->label="GT"; $$->next_node.push_back($1);$$->next_node.push_back($3);}
          | logical GTE logical  { $$ = new node;$$->label="GTE"; $$->next_node.push_back($1);$$->next_node.push_back($3);}
          | logical OR logical   { $$ = new node;$$->label="OR"; $$->next_node.push_back($1);$$->next_node.push_back($3);}
          | logical EQ logical   {$$ = new node; $$->label="EQ"; $$->next_node.push_back($1);$$->next_node.push_back($3);}
          | logical NEQ logical  { $$ = new node;$$->label="NEQ"; $$->next_node.push_back($1);$$->next_node.push_back($3);}
;

%%
