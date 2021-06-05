#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include "parser.hpp"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Value.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetMachine.h"

#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
using namespace std;

extern int yylex();
extern int yyparse();
extern vector<string> symb;
extern struct node *nodes;
extern string* lines;
extern bool _error;
void print_graph(struct node* nodes,string i,llvm::Module TheModule);
struct node{
  string str;
  string shape;
  string label;
  string id;
  string con;
  llvm::Value *value;
  vector<struct node*> next_node;
};
