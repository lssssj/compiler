#pragma once

#include "koopa.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <iostream>
#include <ostream>
#include <scoped_allocator>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>


enum class UnaryOP {
  PLUS, NEG, NOT
};

enum class BinaryOP {
  MUL, DIV, MOD, ADD, SUB
};

enum class RelOP {
  LE, LT, GT, GE, EQ, NEQ
};

enum class LogicalOP {
  AND, OR
};

enum class BType {
  INT,
};

struct Var {
  BType type;
  std::string name;
  bool exited;
  bool constant;
  int val;
};

class SymbolTable {
  public:
    using Scopes = std::stack<std::unordered_map<std::string, Var>>;
    using Scope = std::unordered_map<std::string, Var>;
    Scopes scopes;

    void enterScope() {
      Scope s;
      scopes.push(s);
    }

    void exitScope() {
      scopes.pop();
    }

    // true 插入成功，false 插入失败意味作用域存在相同的名字了。
    bool insert(std::string ident, Var value) {
      Var old_value = lookup(ident);
      if (old_value.exited) {
        return false;
      }
      Scope& map = scopes.top();
      map[ident] = value;
      return true;
    }

    Var lookup(std::string ident) {
      Scope map = scopes.top();
      if (map.find(ident) == map.end()) {
        Var res;
        res.exited = false;
        return res;
      }
      return map[ident];
    }

    Var probe(std::string ident) {
      Scopes clone = scopes;
      while (!clone.empty()) {
        Scope map = clone.top();
        if (map.find(ident) != map.end()) {
          return map[ident];
        }
        clone.pop();
      }
      Var res{.exited = false};
      return res;
    }
};


class Environemt {
  public:
    Environemt() : temp_var(0), is_var(false), block_var(1) {}
    int temp_var;
    bool is_var;
    int block_var;
    std::ostringstream code;
    SymbolTable table;
    std::string block;

    std::string NewTempVar() {
      return "%" + std::to_string(temp_var++);
    }

    void NewBlockName() {
      block = "_" + std::to_string(block_var++);
    }
};

class CodeGen {
  public:
    static std::string emitAdd(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = add " + l + ", " + r + "\n";
    }

    static std::string emitSub(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = sub " + l + ", " + r + "\n";
    }

    static std::string emitMul(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = mul " + l + ", " + r + "\n";
    }

    static std::string emitDiv(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = div " + l + ", " + r + "\n";
    }

    static std::string emitMod(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = mod " + l + ", " + r + "\n";
    }

    static std::string emitEq(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = eq " + l + ", " + r + "\n";
    }

    static std::string emitNe(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = ne " + l + ", " + r + "\n";
    }

    static std::string emitLt(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = lt " + l + ", " + r + "\n";
    }

    static std::string emitGt(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = gt " + l + ", " + r + "\n";
    }

    static std::string emitLe(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = le " + l + ", " + r + "\n";
    }

    static std::string emitGe(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = ge " + l + ", " + r + "\n";
    }

    static std::string emitOr(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = or " + l + ", " + r + "\n";
    }

    static std::string emitAnd(std::string ret, std::string l, std::string r) {
      return "  " + ret + " = and " + l + ", " + r + "\n";
    }

    static std::string emitAlloc(std::string ret, std::string name) {
      return "  " + ret + " = alloc " + name + "\n";
    }

    static std::string emitLoad(std::string ret, std::string name) {
      return "  " + ret + " = load " + name + "\n";
    }

    static std::string emitStore(std::string value, std::string name) {
      return "  store " + value + ", " + name + "\n";
    }
};

class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual void Dump(int ident) const = 0;
    virtual std::string DumpIR(Environemt &env) const = 0;
    virtual int DumpExp(Environemt &env) const {
      assert(false);
      return -1;
    }
};

class CompUnitAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> func_def;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "CompUnitAST {\n";
      func_def->Dump(ident + 2);
      std::cout << id << "}";
    }

    std::string DumpIR(Environemt &env) const override {
      return func_def->DumpIR(env);
    }
};

class FuncDefAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> func_type;
    std::string ident;
    std::unique_ptr<BaseAST> block;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "FuncDefAST { ";
      func_type->Dump(0);
      std::cout << ", " << ident << ", \n";
      block->Dump(ident + 2);
      std::cout << id << "}\n";
    }

    std::string DumpIR(Environemt &env) const override {
      env.code << "fun @" + ident + "(): ";
      env.code << func_type->DumpIR(env);
      env.code << " {\n";
      env.code << "%entry:\n";
      block->DumpIR(env);
      env.code << "}";
      return env.code.str();
    }
};

class FuncTypeAST : public BaseAST {
  public:
    std::string _type;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "FuncTypeAST { " << _type << " }";
    }

    std::string DumpIR(Environemt &env) const override {
      if (_type == "int") {
        return "i32";
      }
      return _type;
    }

};

class BlockAST : public BaseAST {
  public:
    std::vector<std::unique_ptr<BaseAST>> asts;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "BlockAST { \n";
      for (const auto &ast : asts) {
        ast->Dump(ident + 2);
        std::cout << std::endl;
      } 
      std::cout << id << "}\n";
    }

    std::string DumpIR(Environemt &env) const override {
      env.table.enterScope();
      env.NewBlockName();
      for (const auto &ast : asts) {
        ast->DumpIR(env);
      } 
      env.table.exitScope();
      return "";
    }
};

class ReturnStmtAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> ast;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "ReturnStmtAST {\n";
      ast->Dump(ident + 2);
      std::cout << id << "}";
    }

    std::string DumpIR(Environemt &env) const override {
      std::string val = ast->DumpIR(env);
      env.code << "  ret " << val << "\n";
      return "";
    }
};

class AssignStmtAST : public BaseAST {
  public:
    std::string name;
    std::unique_ptr<BaseAST> val;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "AssignStmtAST { \n";
      std::cout << id << "  " << "name=" << name << std::endl;
      val->Dump(ident + 2);
      std::cout << id << "}";
    }

    std::string DumpIR(Environemt &env) const override {
      Var value = env.table.probe(name);
      assert(!value.constant);
      std::string tmp = val->DumpIR(env);
      env.code << CodeGen::emitStore(tmp, value.name);
      return "";
    }
};

class DefAST : public BaseAST {
  public:
    std::string name;
    std::unique_ptr<BaseAST> init;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "DefAST { \n";
      std::cout << id << "  " << "name=" << name << std::endl;
      if (init != nullptr) {
        init->Dump(ident + 2);
      }
      std::cout << id << "}";
    }

    std::string DumpIR(Environemt &env) const override {
      if (init != nullptr) {
        return init->DumpIR(env);
      }
      return "";
    }

    int DumpExp(Environemt &env) const override {
      // 变量不能被Dump;
      assert(false);
    }
};

class ConstDefAST : public BaseAST {
  public:
    std::string name;
    std::unique_ptr<BaseAST> init;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "ConstDefAST { \n";
      std::cout << id << "  " << "name=" << name << std::endl;
      if (init != nullptr) {
        init->Dump(ident + 2);
      }
      std::cout << id << "}";
    }

    std::string DumpIR(Environemt &env) const override {
      return "";
    }

    int DumpExp(Environemt &env) const override {
      int val = init->DumpExp(env);
      Var value{.type=BType::INT, .exited=true, .constant=true, .val=val};
      assert(env.table.insert(name, value));
      return val;
    }
};

class ConstDeclAST : public BaseAST {
  public:
    BType type;
    std::vector<std::unique_ptr<ConstDefAST>> defVars;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "ConstDeclAST { \n";
      std::cout << id << "  ";
      switch (type) {
        case BType::INT:
          std::cout << "type=int" << std::endl;
          break;
      }
      for (const auto &ast : defVars) {
        ast->Dump(ident + 2);
        std::cout << std::endl;
      }
      std::cout << id << "}";
    }

    std::string DumpIR(Environemt &env) const override {
      DumpExp(env);
      return "";
    }

    int DumpExp(Environemt &env) const override {
      for (const auto & ast : defVars) {
        ast->DumpExp(env);
      }  
      return -1;    
    }
};

class DeclAST : public BaseAST {
  public:
    BType type;
    std::vector<std::unique_ptr<DefAST>> defVars;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "DeclAST { \n";
      std::cout << id << "  ";
      switch (type) {
        case BType::INT:
          std::cout << "type=int" << std::endl;
          break;
      }
      for (const auto &ast : defVars) {
        ast->Dump(ident + 2);
        std::cout << std::endl;
      }
      std::cout << id << "}";
    }

    std::string DumpIR(Environemt &env) const override {
      switch (type) {
        case BType::INT: {
          for (const auto & ast : defVars) {
            std::string ret = ast->DumpIR(env);
            Var value{.type=type, .name="@" + ast->name + env.block, .constant=false};
            env.code << CodeGen::emitAlloc(value.name, "i32");
            if (ret != "") {
              env.code << CodeGen::emitStore(ret, value.name);
            }
            env.table.insert(ast->name, value);
          }
        }
        return "";
      }
    }
};

class IdentfierAST : public BaseAST {
  public:
    std::string name;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "IdentfierAST: " << name << "\n";
    }

    std::string DumpIR(Environemt &env) const override {
      Var value = env.table.probe(name);
      if (value.constant) {
        return std::to_string(value.val);
      }
      std::string tmp = env.NewTempVar();
      env.code << CodeGen::emitLoad(tmp, value.name);
      return tmp;
    }

    int DumpExp(Environemt &env) const override {
      Var value = env.table.probe(name);
      if (value.type != BType::INT) {
        assert(false);
      }
      return value.val;
    }
};

class NumberAST : public BaseAST {
  public:
    int val;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << val << "\n";
    }

    std::string DumpIR(Environemt &env) const override {
      return std::to_string(val);
    }

    int DumpExp(Environemt &env) const override {
      return val;
    }
};

class PrimaryExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> val;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id <<  "PrimaryExpAST {\n";
      val->Dump(ident + 2);
      std::cout << id << "}\n";
    }

    std::string DumpIR(Environemt &env) const override {
      return val->DumpIR(env);
    }

    int DumpExp(Environemt &env) const override {
      return val->DumpExp(env);
    }
};

class UnaryExpAST : public BaseAST {
  public:
    UnaryOP op;
    std::unique_ptr<BaseAST> child;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "UnaryExpAST {\n";
      switch (op) {
        case UnaryOP::NEG:
          std::cout << id <<  "-\n";
          break;
        case UnaryOP::NOT:
          std::cout << id << "!\n";
          break;
        case UnaryOP::PLUS:
        default:
          break;          
      }
      child->Dump(ident + 2);
      std::cout << id << "}\n";
    }

    std::string DumpIR(Environemt &env) const override {
      if (op == UnaryOP::PLUS) {
        return child->DumpIR(env);
      }
      std::string ret_code;
      std::string child_var = child->DumpIR(env);
      std::string ret_var = env.NewTempVar();
      switch (op) {
        case UnaryOP::NEG:
          ret_code += CodeGen::emitSub(ret_var, std::to_string(0), child_var);
          break;
        case UnaryOP::NOT:
          ret_code += CodeGen::emitEq(ret_var, std::to_string(0), child_var);
          break;
        default:
          break;
      }
      env.code << ret_code;
      return ret_var;
    }

    int DumpExp(Environemt &env) const override {
      int val = child->DumpExp(env);
      switch (op) {
        case UnaryOP::PLUS:
          return val;
        case UnaryOP::NEG:
          return -val;
        case UnaryOP::NOT:
          if (val == 0) {
            return 1;
          }
          return 0;
      }
    }
};

class BinaryExpAST : public BaseAST {
  public:
    BinaryOP op;
    std::unique_ptr<BaseAST> left;
    std::unique_ptr<BaseAST> right;  

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "BinaryExpAST {\n";
      switch (op) {
        case BinaryOP::ADD:
          std::cout << id << "+\n";
          break;
        case BinaryOP::SUB:
          std::cout << id << "-\n";
          break;
        case BinaryOP::MOD:
          std::cout << id << "%\n";
          break;
        case BinaryOP::MUL:
          std::cout << id << "*\n";
          break;  
        case BinaryOP::DIV:
          std::cout << id << "/\n";
          break;  
      }
      left->Dump(ident + 2);
      right->Dump(ident + 2);
      std::cout << id << "}\n";
    }

    std::string DumpIR(Environemt &env) const override {
      std::string ret_code;
      std::string left_var = left->DumpIR(env);
      std::string right_var = right->DumpIR(env);
      std::string ret_var = env.NewTempVar();
      
      switch (op) {
        case BinaryOP::ADD:
          ret_code += CodeGen::emitAdd(ret_var, left_var, right_var);
          break;
        case BinaryOP::SUB:
          ret_code += CodeGen::emitSub(ret_var, left_var, right_var);
          break;  
        case BinaryOP::MUL:
          ret_code += CodeGen::emitMul(ret_var, left_var, right_var);
          break;
        case BinaryOP::DIV:
          ret_code += CodeGen::emitDiv(ret_var, left_var, right_var);
          break;
        case BinaryOP::MOD:
          ret_code += CodeGen::emitMod(ret_var, left_var, right_var);
          break;      
      }
      env.code << ret_code;
      return ret_var;
    }

    int DumpExp(Environemt &env) const override {
      int lval = left->DumpExp(env);
      int rval = right->DumpExp(env);
      switch (op) {
        case BinaryOP::ADD:
          return lval + rval;
        case BinaryOP::SUB:
          return lval - rval;  
        case BinaryOP::MUL:
          return lval * rval;
        case BinaryOP::DIV:
          return lval / rval;
        case BinaryOP::MOD:
          return lval % rval;    
      }
    }
};

class RelExpAST : public BaseAST {
  public:
    RelOP op;
    std::unique_ptr<BaseAST> left;
    std::unique_ptr<BaseAST> right;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "RelExpAST {\n";
      switch (op) {
        case RelOP::EQ:
          std::cout << id << "=\n";
          break;
        case RelOP::NEQ:
          std::cout << id << "!=\n";
          break;
        case RelOP::LT:
          std::cout << id << "<\n";
          break;
        case RelOP::LE:
          std::cout << id << "<=\n";
          break;
        case RelOP::GT:
          std::cout << id << ">\n";
          break;
        case RelOP::GE:
          std::cout << id << ">=\n";
          break;
      }
      left->Dump(ident + 2);
      right->Dump(ident + 2);
      std::cout << id << "}\n";
    }

    std::string DumpIR(Environemt &env) const override {
      std::string ret_code;
      std::string left_var = left->DumpIR(env);
      std::string right_var = right->DumpIR(env);
      std::string ret_var = env.NewTempVar();
      switch (op) {
        case RelOP::EQ:
          ret_code += CodeGen::emitEq(ret_var, left_var, right_var);
          break;
        case RelOP::NEQ:
          ret_code += CodeGen::emitNe(ret_var, left_var, right_var);
          break;  
        case RelOP::LT:
          ret_code += CodeGen::emitLt(ret_var, left_var, right_var);
          break;
        case RelOP::GT:
          ret_code += CodeGen::emitGt(ret_var, left_var, right_var);
          break;
        case RelOP::LE:
          ret_code += CodeGen::emitLe(ret_var, left_var, right_var);
          break;
        case RelOP::GE:
          ret_code += CodeGen::emitGe(ret_var, left_var, right_var);
          break;        
      }
      env.code << ret_code;
      return ret_var;
    }

    int DumpExp(Environemt &env) const override {
      int lval = left->DumpExp(env);
      int rval = right->DumpExp(env);
      switch (op) {
        case RelOP::EQ:
          return lval == rval;
        case RelOP::NEQ:
          return lval != rval;  
        case RelOP::LT:
          return lval < rval;
        case RelOP::GT:
          return lval > rval;
        case RelOP::LE:
          return lval <= rval;
        case RelOP::GE:
          return lval >= rval;  
      }
    }
}; 

class LogicalExpAST : public BaseAST {
  public:
    LogicalOP op;
    std::unique_ptr<BaseAST> left;
    std::unique_ptr<BaseAST> right;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "LogicalExpAST {\n";
      switch (op) {
        case LogicalOP::AND:
          std::cout << id << "&&\n";
          break;
        case LogicalOP::OR:
          std::cout << id << "||\n";  
      }
      left->Dump(ident + 2);
      right->Dump(ident + 2);
      std::cout << id << "}\n";
    }

    // int 逻辑or  ->  两个数先or, 再判断结果是否 ne 0
    // int 逻辑and ->  两个数先ne 0, 再对两个结果and，最后即为结果
    std::string DumpIR(Environemt &env) const override {
      std::string ret_code;
      std::string left_var = left->DumpIR(env);
      std::string right_var = right->DumpIR(env);
      std::string ret_var = env.NewTempVar();
      switch (op) {
        case LogicalOP::OR: {
          std::string r1 = env.NewTempVar();
          ret_code += CodeGen::emitOr(r1, left_var, right_var);
          ret_code += CodeGen::emitNe(ret_var, std::to_string(0), r1);
          break;
        }
        case LogicalOP::AND: {
          std::string r1 = env.NewTempVar();
          std::string r2 = env.NewTempVar();
          ret_code += CodeGen::emitNe(r1, std::to_string(0), left_var);
          ret_code += CodeGen::emitNe(r2, std::to_string(0), right_var);
          ret_code += CodeGen::emitAnd(ret_var, r1, r2);
          break;
        }
      }
      env.code << ret_code;
      return ret_var;
    }

    int DumpExp(Environemt &env) const override {
      int lval = left->DumpExp(env);
      int rval = right->DumpExp(env);
      switch (op) {
        case LogicalOP::OR: {
          return lval || rval;
          break;
        }
        case LogicalOP::AND: {
          return lval && rval;
        }
      }
    }
};

