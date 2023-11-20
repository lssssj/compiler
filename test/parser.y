%{

#include <iostream>
#include <memory>
#include <string>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(const char *s);

using namespace std;



struct E_ast {
    string name;


    friend ostream & operator<<( ostream & os, const E_ast & c) {
        os << "E_ast{name=" << (c.name) <<  "}" << endl;
        return os;
    }
};

struct W_ast {
    string name;


    friend ostream & operator<<( ostream & os, const W_ast & c) {
        os << "W_ast{name=" << (c.name) <<  "}" << endl;
        return os;
    }
};

struct Program {
    struct E_ast e;
    struct W_ast w;
    string name;


    friend ostream & operator<<( ostream & os, const Program & p) {
        os << "Program{name=" << (p.name) << "," << (p.e) << ", " << (p.w) << "}" << endl;
        return os;
    }
};

Program * res;

%}

%union {
  struct Program *p;  
  std::string *str;
  struct E_ast *e;
  struct W_ast *w;
}

%token <str> WHILE
%token <str> ELSE
%token <str> CHAR

%type <p> Program
%type <e> Else
%type <w> While

%%

Program : While Else CHAR {
    Program *p = new Program();
    p->w = *$1;
    std::cout << p->w << std::endl;
    p->e = *$2;
    std::cout << p->e << std::endl;
    p->name = *$3;
    $$ = p;
    res = p;
}

While : WHILE { 
    W_ast *w = new W_ast();
    w->name =  *$1;
    $$ = w;
}

Else : ELSE { 
    E_ast *e = new E_ast();
    e->name =  *$1;
    $$ = e;
}

%%

int main() {
  yyparse();
  Program *p = res;
  cout << *p << endl;
  cout << p << " " << yylval.p << endl;


}


// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(const char *s) {
  cerr << "error: " << s << endl;
}
