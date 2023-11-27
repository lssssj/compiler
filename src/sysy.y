%code requires {
  #include <memory>
  #include <string>
  #include "ast.hpp"
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <algorithm>
#include "ast.hpp"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  BType type;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token OR AND LE LT GE GT EQ NEQ PLUS SUB MUL DIV MOD NOT CONST

%token INT RETURN
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Number
%type <ast_val> Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <ast_val> BlockItem ConstDef VarDef Decl ConstDecl VarDecl
%type <type> BType;

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->_type = string("int");
    $$ = ast;
  }
  ;

Block
  : '{' BlockItem '}' {
    auto block = (BlockAST*) $2;
    std::reverse(block->asts.begin(), block->asts.end());
    $$ = block;
  }
  ;

BlockItem
  : Stmt {
    auto ast = new BlockAST();
    ast->asts.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | Decl {
    auto ast = new BlockAST();
    ast->asts.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  }
  | Stmt BlockItem {
    auto block = (BlockAST*)($2);
    auto ast = unique_ptr<BaseAST>($1);
    block->asts.push_back(std::move(ast));
    $$ = block;
  }
  | Decl BlockItem {
    auto block = (BlockAST*)($2);
    auto ast = unique_ptr<BaseAST>($1);
    block->asts.push_back(std::move(ast));
    $$ = block;
  }

Decl
  : ConstDecl {
    auto ast = (ConstDeclAST*) $1;
    std::reverse(ast->defVars.begin(), ast->defVars.end());
    $$ = ast;
  }  
  | VarDecl {
    auto ast = (DeclAST*) $1;
    std::reverse(ast->defVars.begin(), ast->defVars.end());
    $$ = ast;
  }
  ;

ConstDecl
  : CONST BType ConstDef ';' {
    auto decl = (ConstDeclAST*)($3);
    decl->type = $2;
    $$ = decl;
  } 
  ;

ConstDef
  : IDENT '=' Exp {
    auto decl = new ConstDeclAST();
    auto ast = new ConstDefAST();
    ast->name = *unique_ptr<string>($1);
    ast->init = unique_ptr<BaseAST>($3);
    decl->defVars.push_back(std::unique_ptr<ConstDefAST>(ast));
    $$ = decl;
  }
  | IDENT '=' Exp ',' ConstDef {
    auto decl = (ConstDeclAST*)($5);
    auto ast = new ConstDefAST();
    ast->name = *unique_ptr<string>($1);
    ast->init = unique_ptr<BaseAST>($3);
    decl->defVars.push_back(std::unique_ptr<ConstDefAST>(ast));
    $$ = decl;
  }
  ;

VarDecl
  : BType VarDef ';' {
    auto decl = (DeclAST*)($2);
    decl->type = $1;
    $$ = decl;
  } 
  ;

VarDef
  : IDENT {
    auto decl = new DeclAST();
    auto ast = new DefAST();
    ast->name = *unique_ptr<string>($1);
    decl->defVars.push_back(std::unique_ptr<DefAST>(ast));
    $$ = decl;
  }
  | IDENT '=' Exp {
    auto decl = new DeclAST();
    auto ast = new DefAST();
    ast->name = *unique_ptr<string>($1);
    ast->init = unique_ptr<BaseAST>($3);
    decl->defVars.push_back(std::unique_ptr<DefAST>(ast));
    $$ = decl;
  } 
  | IDENT ',' VarDef {
    auto decl = (DeclAST*)$3;
    auto ast = new DefAST();
    ast->name = *unique_ptr<string>($1);
    decl->defVars.push_back(std::unique_ptr<DefAST>(ast));
    $$ = decl;
  }
  | IDENT '=' Exp ',' VarDef {
    auto decl = (DeclAST*)$5;
    auto ast = new DefAST();
    ast->name = *unique_ptr<string>($1);
    ast->init = unique_ptr<BaseAST>($3);
    decl->defVars.push_back(std::unique_ptr<DefAST>(ast));
    $$ = decl;
  }
  ;

BType
  : INT {
    $$ = BType::INT;
  }
  ;
 
Stmt
  : RETURN Exp ';' {
    auto ast = new ReturnStmtAST();
    ast->ast = unique_ptr<BaseAST>($2); 
    $$ = ast;
  }
  | IDENT '=' Exp ';' {
    auto ast = new AssignStmtAST();
    ast->name = *unique_ptr<std::string>($1);
    ast->val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  ;

Exp
  : LOrExp  {
    $$ = $1;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST();
    ast->val = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST();
    ast->val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | IDENT {
    auto ast = new IdentfierAST();
    ast->name = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp {
    $$ = $1;
  } 
  | PLUS UnaryExp {
    auto ast = new UnaryExpAST();
    ast->child = unique_ptr<BaseAST>($2);
    ast->op = UnaryOP::PLUS;
    $$ = ast;
  } 
  | SUB UnaryExp {
    auto ast = new UnaryExpAST();
    ast->child = unique_ptr<BaseAST>($2);
    ast->op = UnaryOP::NEG;
    $$ = ast;
  }
  | NOT UnaryExp {
    auto ast = new UnaryExpAST();
    ast->child = unique_ptr<BaseAST>($2);
    ast->op = UnaryOP::NOT;
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp {
    $$ = $1;
  } 
  | MulExp MUL UnaryExp {
    auto ast = new BinaryExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = BinaryOP::MUL;
    $$ = ast;
  }
  | MulExp DIV UnaryExp {
    auto ast = new BinaryExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = BinaryOP::DIV;
    $$ = ast;
  }
  | MulExp MOD UnaryExp {
    auto ast = new BinaryExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = BinaryOP::MOD;
    $$ = ast;
  }
  ;

AddExp
  : MulExp {
    $$ = $1;
  }
  | AddExp PLUS MulExp {
    auto ast = new BinaryExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = BinaryOP::ADD;
    $$ = ast;
  } 
  | AddExp SUB MulExp {
    auto ast = new BinaryExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = BinaryOP::SUB;
    $$ = ast;
  } 
  ;

RelExp
  : AddExp {
    $$ = $1;
  }  
  | RelExp LE AddExp {
    auto ast = new RelExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = RelOP::LE;
    $$ = ast;
  }
  | RelExp GE AddExp {
    auto ast = new RelExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = RelOP::GE;
    $$ = ast;
  }
  | RelExp LT AddExp {
    auto ast = new RelExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = RelOP::LT;
    $$ = ast;
  }
  | RelExp GT AddExp {
    auto ast = new RelExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = RelOP::GT;
    $$ = ast;
  }
  ;

EqExp
  : RelExp {
    $$ = $1;
  } 
  | EqExp EQ RelExp {
    auto ast = new RelExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = RelOP::EQ;
    $$ = ast;
  }
  | EqExp NEQ RelExp {
    auto ast = new RelExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = RelOP::NEQ;
    $$ = ast;
  }
  ;

LAndExp
  : EqExp {
    $$ = $1;
  }
  | LAndExp AND EqExp {
    auto ast = new LogicalExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = LogicalOP::AND;
    $$ = ast;
  }
  ;

LOrExp
  : LAndExp {
    $$ = $1;
  } 
  | LOrExp OR LAndExp {
    auto ast = new LogicalExpAST();
    ast->left = unique_ptr<BaseAST>($1);
    ast->right = unique_ptr<BaseAST>($3);
    ast->op = LogicalOP::OR;
    $$ = ast;
  } 
  ;  

Number
  : INT_CONST {
    auto ast = new NumberAST();
    ast->val = $1;
    $$ = ast;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
