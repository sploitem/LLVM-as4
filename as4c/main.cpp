#include "main.hpp"




// using namespace llvm;

static llvm::LLVMContext TheContext;
static llvm::IRBuilder<> TheBuilder(TheContext);
static llvm::Module* TheModule;

static std::map<std::string, llvm::Value*> TheSymbolTable;

llvm::Value* numericConstant(float val) {
  return llvm::ConstantFP::get(TheContext, llvm::APFloat(val));
}
llvm::Value* variableValue(const std::string& name) {
  llvm::Value* val = TheSymbolTable[name];
  //cout << "test 1" << endl;
  if (!val) {
    std::cerr << "Invalid variable name: " << name << std::endl;
    return NULL;
  }
  // cout << "test 2" << endl;
  return TheBuilder.CreateLoad(val,name.c_str());
  
}
llvm::Value* binaryOperation(llvm::Value* lhs, llvm::Value* rhs, char op) {
  if (!lhs || !rhs) {
    return NULL;
  }

  switch (op) {
  case '+':
    return TheBuilder.CreateFAdd(lhs, rhs, "addtmp");
  case '-':
    return TheBuilder.CreateFSub(lhs, rhs, "subtmp");
  case '*':
    return TheBuilder.CreateFMul(lhs, rhs, "multmp");
  case '/':
    return TheBuilder.CreateFDiv(lhs, rhs, "divtmp");
  case '<':
    lhs = TheBuilder.CreateFCmpULT(lhs, rhs, "lttmp");
    return TheBuilder.CreateUIToFP(lhs, llvm::Type::getFloatTy(TheContext), "booltmp");
  case '>':
    lhs = TheBuilder.CreateFCmpUGT(lhs, rhs, "fcmp");
    return TheBuilder.CreateUIToFP(lhs, llvm::Type::getFloatTy(TheContext), "booltmp");
  case '~':
    lhs = TheBuilder.CreateFCmpUGE(lhs, rhs, "gtetmp");
    return TheBuilder.CreateUIToFP(lhs, llvm::Type::getFloatTy(TheContext), "booltmp");
  default:
    std::cerr << "Invalid operator:" << op << std::endl;
    return NULL;
  }
}
llvm::AllocaInst* generateEntryBlockAlloca(std::string& name) {
  llvm::Function* currFn =
    TheBuilder.GetInsertBlock()->getParent();
  llvm::IRBuilder<> tmpBuilder(&currFn->getEntryBlock(),
    currFn->getEntryBlock().begin());

  return tmpBuilder.CreateAlloca(llvm::Type::getFloatTy(TheContext), 0, name.c_str());
}
llvm::Value* assignmentStatement(std::string& lhs, llvm::Value* rhs) {
  if (!rhs) {
    return NULL;
  }
  if (!TheSymbolTable.count(lhs)) {
    // Allocate lhs
    TheSymbolTable[lhs] = generateEntryBlockAlloca(lhs);
  }
  return TheBuilder.CreateStore(rhs, TheSymbolTable[lhs]);
}
llvm::Value* ifElseStatement() {
  llvm::Value* cond = binaryOperation(variableValue("b"), numericConstant(8), '<');
  if (!cond) {
    return NULL;
  }
  cond = TheBuilder.CreateFCmpONE(cond, numericConstant(0), "ifcond");

  llvm::Function* currFn =
    TheBuilder.GetInsertBlock()->getParent();
  llvm::BasicBlock* ifBlock =
    llvm::BasicBlock::Create(TheContext, "ifBlock", currFn);
  llvm::BasicBlock* elseBlock =
    llvm::BasicBlock::Create(TheContext, "elseBlock");
  llvm::BasicBlock* contBlock =
    llvm::BasicBlock::Create(TheContext, "contBlock");

  TheBuilder.CreateCondBr(cond, ifBlock, elseBlock);
  TheBuilder.SetInsertPoint(ifBlock);

  /* If block */
  llvm::Value* aTimesB = binaryOperation(
    variableValue("a"),
    variableValue("b"),
    '*'
  );
  std::string var("c");
  llvm::Value* ifBlockStatement = assignmentStatement(var, aTimesB);
  TheBuilder.CreateBr(contBlock);

  /* Else block */
  currFn->getBasicBlockList().push_back(elseBlock);
  TheBuilder.SetInsertPoint(elseBlock);
  llvm::Value* aPlusB = binaryOperation(
    variableValue("a"),
    variableValue("b"),
    '+'
  );
  llvm::Value* elseBlockStatement = assignmentStatement(var, aPlusB);
  TheBuilder.CreateBr(contBlock);

  currFn->getBasicBlockList().push_back(contBlock);
  TheBuilder.SetInsertPoint(contBlock);

  return contBlock;
}
void generateObjCode(const std::string& filename) {
  std::string targetTriple = llvm::sys::getDefaultTargetTriple();

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  std::string error;
  const llvm::Target* target =
    llvm::TargetRegistry::lookupTarget(targetTriple, error);
  if (!target) {
    std::cerr << error << std::endl;
  } else {
    std::string cpu = "generic";
    std::string features = "";
    llvm::TargetOptions options;
    llvm::TargetMachine* targetMachine =
      target->createTargetMachine(targetTriple, cpu, features,
        options, llvm::Optional<llvm::Reloc::Model>());

    TheModule->setDataLayout(targetMachine->createDataLayout());
    TheModule->setTargetTriple(targetTriple);

    std::error_code ec;
    llvm::raw_fd_ostream file(filename, ec, llvm::sys::fs::F_None);
    if (ec) {
      std::cerr << "Could not open output file: " << ec.message() << std::endl;
    } else {
      llvm::TargetMachine::CodeGenFileType type = llvm::TargetMachine::CGFT_ObjectFile;
      llvm::legacy::PassManager pm;
      if (targetMachine->addPassesToEmitFile(pm, file, NULL, type)) {
        std::cerr << "Unable to emit target code" << std::endl;
      } else {
        pm.run(*TheModule);
        file.close();
      }
    }
  }
}
void doOptimizations(llvm::Function* fn) {
  llvm::legacy::FunctionPassManager* fpm =
    new llvm::legacy::FunctionPassManager(TheModule);

  fpm->add(llvm::createPromoteMemoryToRegisterPass());
  // fpm->add(llvm::createInstructionCombiningPass());
  fpm->add(llvm::createReassociatePass());
  fpm->add(llvm::createGVNPass());
  fpm->add(llvm::createCFGSimplificationPass());

  fpm->run(*fn);
}

llvm::Value* assignment_leftdown(struct node* nodes){
    //cout << "come to assignment leftdown" << endl;
    if(nodes->shape == "box"){
     // cout << "come to assignment leftdown box" << endl;
      if(nodes->label[1]== 'd'){
       //   cout << "come to assignment leftdown box if" << endl;
        return variableValue(nodes->str);
        // cout << nodes->str << endl;
      }else{
        //cout << "come to here" << endl;
        float temp = atof(nodes->str.c_str());
        return numericConstant(temp);
      }
    }
    if(nodes->next_node[0]->label!="box"){
       //cout << "come to assignment_leftdown if" << endl;
       if(nodes->next_node[1]->label[1]== 'd'){
         // cout << "come to assignment_leftdown if if" << endl;
          if(nodes->label== "PLUS"){
           //  cout << "come to assignment_leftdown box if plus" << endl;
            // cout << nodes->next_node[0]->shape << endl;
            return binaryOperation(assignment_leftdown(nodes->next_node[0]),variableValue(nodes->next_node[1]->str),'+');
          }else if(nodes->label =="TIMES"){
             //cout << "come to assignment_leftdown box if times" << endl;
            return binaryOperation(assignment_leftdown(nodes->next_node[0]),variableValue(nodes->next_node[1]->str),'*');
          }else if(nodes->label =="MINUS"){
             //cout << "come to assignment_leftdown box if minus" << endl;
            return binaryOperation(assignment_leftdown(nodes->next_node[0]),variableValue(nodes->next_node[1]->str),'-');
          }else if(nodes->label =="DIVIDEDBY"){
             //cout << "come to assignment_leftdown box if devidedby" << endl;
            return binaryOperation(assignment_leftdown(nodes->next_node[0]),variableValue(nodes->next_node[1]->str),'/');
          }else{
            //cout <<" error 1" << endl;
            return NULL;
          }
        }else{
          // cout << "come to assignment_leftdown box else" << endl;
          float temp = atof(nodes->next_node[1]->str.c_str());
          if(nodes->label== "PLUS"){
            // cout << "come to assignment_leftdown box else plus" << endl;
            return binaryOperation(numericConstant(temp),assignment_leftdown(nodes->next_node[0]),'+');
          }else if(nodes->label =="TIMES"){
             //cout << "come to assignment_leftdown box else times" << endl;
            return binaryOperation(numericConstant(temp),assignment_leftdown(nodes->next_node[0]),'*');
          }else if(nodes->label =="MINUS"){
             //cout << "come to assignment_leftdown box else minus" << endl;
            return binaryOperation(numericConstant(temp),assignment_leftdown(nodes->next_node[0]),'-');
          }else if(nodes->label =="DIVIDEDBY"){
             //cout << "come to assignment_leftdown box else dividedby" << endl;
            return binaryOperation(numericConstant(temp),assignment_leftdown(nodes->next_node[0]),'/');
          }else{
          //  cout <<" error" << endl;
            return NULL;
          }
        }
    }else{
      //cout << "assignment_leftdown else" << endl;
      if(nodes->next_node[0]->label[1]=='d' && nodes->next_node[1]->label[1]=='d'){
        //cout << "test" << endl;
        //cout << "assignment_leftdown else if" << endl;
        llvm::Value* temp = NULL;
        if(nodes->label =="TIMES"){ 
          //cout << "assignment_leftdown else if plus" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[1]->str),variableValue(nodes->next_node[0]->str),'*');
        }else if(nodes->label =="PLUS"){
          //cout << "assignment_leftdown else if plus" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[1]->str),variableValue(nodes->next_node[0]->str),'+');
        }else if(nodes->label =="MINUS"){
          //cout << "assignment_leftdown else minus" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[1]->str),variableValue(nodes->next_node[0]->str),'-');
        }else if(nodes->label =="DIVIDEDBY"){
          //cout << "assignment_leftdown else if dividedby" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[1]->str),variableValue(nodes->next_node[0]->str),'/');
        }else{
          cout << "Erorr in assignment_leftdown" << endl;
          return NULL;
        }
        return temp;
      }else if(nodes->next_node[0]->label[1] !='d' && nodes->next_node[1]->label[1]=='d'){
       // cout << "assignment_leftdown else else if" << endl;
        float temp2 = atof(nodes->next_node[0]->str.c_str());
        llvm::Value* temp = NULL;
        if(nodes->label =="TIMES"){
         // cout << "assignment_leftdown else else if1 times" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[1]->str),numericConstant(temp2),'*');
        }else if(nodes->label =="PLUS"){
          //cout << "assignment_leftdown else else if1 plus" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[1]->str),numericConstant(temp2),'+');
        }else if(nodes->label =="MINUS"){
          //cout << "assignment_leftdown else else if1 minus" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[1]->str),numericConstant(temp2),'-');
        }else if(nodes->label =="DIVIDEDBY"){
          //cout << "assignment_leftdown else else if1 dividedby" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[1]->str),numericConstant(temp2),'/');
        }else{
          //cout << "Erorr in assignment_leftdown" << endl;
          return NULL;
        }
        return temp;
      }else if(nodes->next_node[0]->label[1] =='d' && nodes->next_node[1]->label[1]!='d'){
        float temp2 = atof(nodes->next_node[1]->str.c_str());
        llvm::Value* temp = NULL;
        if(nodes->label =="TIMES"){
          //cout << "assignment_leftdown else else if2 times" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[0]->str),numericConstant(temp2),'*');
        }else if(nodes->label =="PLUS"){
          //cout << "assignment_leftdown else else if2 times" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[0]->str),numericConstant(temp2),'+');
        }else if(nodes->label =="MINUS"){
          //cout << "assignment_leftdown else else if2 times" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[0]->str),numericConstant(temp2),'-');
        }else if(nodes->label =="DIVIDEDBY"){
          //cout << "assignment_leftdown else else if2 times" << endl;
          temp = binaryOperation(variableValue(nodes->next_node[0]->str),numericConstant(temp2),'/');
        }else if(nodes->label == "boolean true"){
          
        }else if(nodes->label == "boolean false"){

        }else{
          //cout << "Erorr in assignment_leftdown" << endl;
          return NULL;
        }
        return temp;
       }else if(nodes->next_node[0]->label[1] !='d' && nodes->next_node[1]->label[1] !='d'){
        float temp1 = atof(nodes->next_node[0]->str.c_str());
        float temp2 = atof(nodes->next_node[1]->str.c_str());
         llvm::Value* temp = NULL;
        if(nodes->label =="TIMES"){
           //cout << "assignment_leftdown else else if3 times" << endl;
           temp = binaryOperation(numericConstant(temp1),numericConstant(temp2),'*');
        }else if(nodes->label =="PLUS"){
          //cout << "assignment_leftdown else else if3 times" << endl;
          temp = binaryOperation(numericConstant(temp1),numericConstant(temp2),'+');
        }else if(nodes->label =="MINUS"){
          //cout << "assignment_leftdown else else if3 times" << endl;
         temp = binaryOperation(numericConstant(temp1),numericConstant(temp2),'-');
        }/* else if(nodes->label =="DIVIDEDBY"){
         temp = binaryOperation((numericConstant(temp1),numericConstant(temp2),'/');
        }*/else{
          //cout << "assignment_leftdown else else if3 times" << endl;
          //cout << "Erorr in assignment_leftdown" << endl;
          return NULL;
        }
        return temp;
      }else{
        //cout << "Erorr in assignment_leftdown " << endl;
        return NULL;
      }
      
    }
}

llvm::Value* print_graph(struct node* nodes,string i, llvm::Module* TheModule){
  if(nodes->label=="Block"){
      //cout << "come into block" << endl;
      for(struct node* i: nodes->next_node){
        print_graph(i,"0",TheModule);
      }
      return NULL;
  }else if(nodes->label=="Assignment"){
      //cout << "come into assignment" << endl;
      if(nodes->next_node[1]->shape == "box" && nodes->next_node[1]->label[1] != 'd' ){
       // cout << "come to assignment box" << endl;
        float num_temp = atof(nodes->next_node[1]->str.c_str());
        if(nodes->next_node[1]->label == "boolean true"){
          llvm::Value* temp = assignmentStatement(nodes->next_node[0]->str,numericConstant(1));
        }else if(nodes->next_node[1]->label == "boolean false"){
          llvm::Value* temp = assignmentStatement(nodes->next_node[0]->str,numericConstant(0));
        }else{
          llvm::Value* value_temp = numericConstant(num_temp);
          llvm::Value* temp = assignmentStatement(nodes->next_node[0]->str,value_temp);
        }
        return NULL;
      }else{
        //cout << "come to assignment not box" << endl;
        llvm::Value* temp = assignmentStatement(nodes->next_node[0]->str,assignment_leftdown(nodes->next_node[1]));
        //cout << "test 402" << endl;
        return temp;
      }
  }else if(nodes->label == "If"){
        // deal with judging
        llvm::Value* cond = NULL;
        if(nodes->next_node[0]->label[1] == 'd'){
            cond = variableValue(nodes->next_node[0]->str);
            if (!cond) {
              return NULL;
            }
            cond = TheBuilder.CreateFCmpONE(cond, numericConstant(0), "ifcond");
        }else if(nodes->next_node[0]->label == "GT"){
            cond = binaryOperation(variableValue("b"), numericConstant(8), '>');
            if (!cond) {
              return NULL;
            }
            cond = TheBuilder.CreateFCmpONE(cond, numericConstant(0), "ifcond");
        }else if(nodes->next_node[0]->label == "GTE"){
            cond = binaryOperation(variableValue(nodes->next_node[0]->next_node[0]->str),variableValue(nodes->next_node[0]->next_node[1]->str) , '~');
            if (!cond) {
              return NULL;
            }
            cond = TheBuilder.CreateFCmpONE(cond, numericConstant(0), "ifcond");
        }else{
          cout << "out of situations in this case" << endl;
        }
        llvm::Function* currFn =
          TheBuilder.GetInsertBlock()->getParent();
        llvm::BasicBlock* ifBlock =
          llvm::BasicBlock::Create(TheContext, "ifBlock", currFn);
        llvm::BasicBlock* elseBlock =
          llvm::BasicBlock::Create(TheContext, "elseBlock");
        llvm::BasicBlock* contBlock =
          llvm::BasicBlock::Create(TheContext, "ifcontBlock");
         if(nodes->next_node[1]->next_node[0]->label == "break"){
           TheBuilder.CreateCondBr(cond, ifBlock, ifBlock);
          TheBuilder.SetInsertPoint(ifBlock);
          //TheBuilder.CreateBr(contBlock); 
          currFn->getBasicBlockList().push_back(contBlock);
          TheBuilder.SetInsertPoint(contBlock);
           return NULL;
         }
         if(nodes->next_node.size()  > 2){
            TheBuilder.CreateCondBr(cond, ifBlock, elseBlock);
            TheBuilder.SetInsertPoint(ifBlock);
            print_graph(nodes->next_node[1],"0",TheModule);
            TheBuilder.CreateBr(contBlock); 
            currFn->getBasicBlockList().push_back(elseBlock);
            TheBuilder.SetInsertPoint(elseBlock);
            print_graph(nodes->next_node[2],"0",TheModule);
             TheBuilder.CreateBr(contBlock); 
          
        }else{
            TheBuilder.CreateCondBr(cond, ifBlock, contBlock);
          TheBuilder.SetInsertPoint(ifBlock);
           
            print_graph(nodes->next_node[1],"0",TheModule);
            TheBuilder.CreateBr(contBlock); 
            //currFn->getBasicBlockList().push_back(contBlock);
            //TheBuilder.SetInsertPoint(contBlock);
        }
       currFn->getBasicBlockList().push_back(contBlock);
       TheBuilder.SetInsertPoint(contBlock);

  }else if(nodes->label == "While"){
        //cout << "come to while" << endl;
        // deal with judging
        llvm::Value* cond = NULL;
        cond = numericConstant(1);

        llvm::Function* currFn =
          TheBuilder.GetInsertBlock()->getParent();
        llvm::BasicBlock* whileCondBlock =
          llvm::BasicBlock::Create(TheContext, "whileCondBlock", currFn);
        llvm::BasicBlock* whileBlock =
          llvm::BasicBlock::Create(TheContext, "whileBlock");
        llvm::BasicBlock* whileContinueBlock =
          llvm::BasicBlock::Create(TheContext, "whileContinueBlock");
        //if(nodes->label == "While"){
            TheBuilder.CreateBr(whileCondBlock); 
            TheBuilder.SetInsertPoint(whileCondBlock);
            TheBuilder.CreateCondBr(cond, whileBlock, whileContinueBlock);
            
            currFn->getBasicBlockList().push_back(whileBlock);
            TheBuilder.SetInsertPoint(whileBlock);
            print_graph(nodes->next_node[1],"0",TheModule);
            TheBuilder.CreateBr(whileContinueBlock); 
            currFn->getBasicBlockList().push_back(whileContinueBlock);
            TheBuilder.SetInsertPoint(whileContinueBlock);
          //  cout << "does it end here" << endl;
        //}else{
            //TheBuilder.CreateBr(whileCondBlock);  
       // }
        
  }else{
     // cout << "error 2" << endl;
      return NULL;     
  }
}

int main(int argc, char const *argv[]) {
  if (!yylex()) {
    TheModule = new llvm::Module("Python compiler",TheContext);

    llvm::FunctionType* mainFnType = llvm::FunctionType::get(
      llvm::Type::getFloatTy(TheContext), false
    );
    llvm::Function* mainFn = llvm::Function::Create(
      mainFnType,
      llvm::GlobalValue::ExternalLinkage,
      "foo",
      TheModule
    );
    llvm::BasicBlock* entryBlock =
        llvm::BasicBlock::Create(TheContext, "entry", mainFn);
    TheBuilder.SetInsertPoint(entryBlock);

    
    print_graph(nodes,"0",TheModule);
    
    TheBuilder.CreateRet(NULL);
    if(argc > 1){
        if(strcmp(argv[1],"-o") == 0){
       //       cout << "op"<< endl;
              doOptimizations(mainFn);
            }
    }
    
    llvm::verifyFunction(*mainFn);
    TheModule->print(llvm::outs(),NULL,TheModule);
    //cout << "here to test 1111" << endl;
    string filename = "outputs.o";
    generateObjCode(filename);
    //cout << "here to test 1122" << endl;
    delete TheModule;
   // cout << "here to test 1222" << endl;
  }
  return 0;
}
