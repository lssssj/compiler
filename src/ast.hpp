#pragma once

#include "koopa.h"
#include <memory>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

#define Int int


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

class Environemt {
  public:
    Environemt() : temp_var(0), is_var(false), ir_ident(0) {}
    int temp_var;
    bool is_var;
    int ir_ident;
    std::ostringstream code;

    std::string NewTempVar() {
      return "%" + std::to_string(temp_var++);
    }
};

class CodeGen {
  public:
    static std::string emitAdd(std::string ret, std::string l, std::string r) {
      return ret + " = add " + l + ", " + r + "\n";
    }

    static std::string emitSub(std::string ret, std::string l, std::string r) {
      return ret + " = sub " + l + ", " + r + "\n";
    }

    static std::string emitMul(std::string ret, std::string l, std::string r) {
      return ret + " = mul " + l + ", " + r + "\n";
    }

    static std::string emitDiv(std::string ret, std::string l, std::string r) {
      return ret + " = div " + l + ", " + r + "\n";
    }

    static std::string emitMod(std::string ret, std::string l, std::string r) {
      return ret + " = mod " + l + ", " + r + "\n";
    }

    static std::string emitEq(std::string ret, std::string l, std::string r) {
      return ret + " = eq " + l + ", " + r + "\n";
    }

    static std::string emitNe(std::string ret, std::string l, std::string r) {
      return ret + " = ne " + l + ", " + r + "\n";
    }

    static std::string emitLt(std::string ret, std::string l, std::string r) {
      return ret + " = lt " + l + ", " + r + "\n";
    }

    static std::string emitGt(std::string ret, std::string l, std::string r) {
      return ret + " = gt " + l + ", " + r + "\n";
    }

    static std::string emitLe(std::string ret, std::string l, std::string r) {
      return ret + " = le " + l + ", " + r + "\n";
    }

    static std::string emitGe(std::string ret, std::string l, std::string r) {
      return ret + " = ge " + l + ", " + r + "\n";
    }

    static std::string emitOr(std::string ret, std::string l, std::string r) {
      return ret + " = or " + l + ", " + r + "\n";
    }

    static std::string emitAnd(std::string ret, std::string l, std::string r) {
      return ret + " = and " + l + ", " + r + "\n";
    }
};

class BaseAST {
  public:
    virtual ~BaseAST() = default;
    virtual void Dump(int ident) const = 0;
    virtual std::string DumpIR(Environemt &env) const = 0;
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
      std::cout << id << "}";
    }

    std::string DumpIR(Environemt &env) const override {
      env.code << "fun @" + ident + "(): ";
      env.code << func_type->DumpIR(env);
      env.code << " {\n";
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
    std::unique_ptr<BaseAST> ast;

    void Dump(int ident) const override {
      std::string id = std::string(ident, ' ');
      std::cout << id << "BlockAST { \n";
      ast->Dump(ident + 2);
      std::cout << id << "}";
    }

    std::string DumpIR(Environemt &env) const override {
      std::string id = std::string(env.ir_ident, ' ');
      env.code << id << "%entry:\n";
      env.ir_ident += 2;
      ast->DumpIR(env);
      env.ir_ident -= 2;
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
      std::string id = std::string(env.ir_ident, ' ');
      env.code << id << "ret " << val << "\n";
      return "";
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
      std::string id = std::string(env.ir_ident, ' ');
      std::string ret_code;
      std::string child_var = child->DumpIR(env);
      std::string ret_var = env.NewTempVar();
      switch (op) {
        case UnaryOP::NEG:
          ret_code += id + CodeGen::emitSub(ret_var, std::to_string(0), child_var);
          break;
        case UnaryOP::NOT:
          ret_code += id + CodeGen::emitEq(ret_var, std::to_string(0), child_var);
          break;
        default:
          break;
      }
      env.code << ret_code;
      return ret_var;
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
      std::string id = std::string(env.ir_ident, ' ');
      std::string ret_code;
      std::string left_var = left->DumpIR(env);
      std::string right_var = right->DumpIR(env);
      std::string ret_var = env.NewTempVar();
      
      switch (op) {
        case BinaryOP::ADD:
          ret_code += id + CodeGen::emitAdd(ret_var, left_var, right_var);
          break;
        case BinaryOP::SUB:
          ret_code += id + CodeGen::emitSub(ret_var, left_var, right_var);
          break;  
        case BinaryOP::MUL:
          ret_code += id + CodeGen::emitMul(ret_var, left_var, right_var);
          break;
        case BinaryOP::DIV:
          ret_code += id + CodeGen::emitDiv(ret_var, left_var, right_var);
          break;
        case BinaryOP::MOD:
          ret_code += id + CodeGen::emitMod(ret_var, left_var, right_var);
          break;      
      }
      env.code << ret_code;
      return ret_var;
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
      std::string id = std::string(env.ir_ident, ' ');
      std::string ret_code;
      std::string left_var = left->DumpIR(env);
      std::string right_var = right->DumpIR(env);
      std::string ret_var = env.NewTempVar();
      switch (op) {
        case RelOP::EQ:
          ret_code += id + CodeGen::emitEq(ret_var, left_var, right_var);
          break;
        case RelOP::NEQ:
          ret_code += id + CodeGen::emitNe(ret_var, left_var, right_var);
          break;  
        case RelOP::LT:
          ret_code += id + CodeGen::emitLt(ret_var, left_var, right_var);
          break;
        case RelOP::GT:
          ret_code += id + CodeGen::emitGt(ret_var, left_var, right_var);
          break;
        case RelOP::LE:
          ret_code += id + CodeGen::emitLe(ret_var, left_var, right_var);
          break;
        case RelOP::GE:
          ret_code += id + CodeGen::emitGe(ret_var, left_var, right_var);
          break;        
      }
      env.code << ret_code;
      return ret_var;
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
      std::string id = std::string(env.ir_ident, ' ');
      std::string ret_code;
      std::string left_var = left->DumpIR(env);
      std::string right_var = right->DumpIR(env);
      std::string ret_var = env.NewTempVar();
      switch (op) {
        case LogicalOP::OR: {
          std::string r1 = env.NewTempVar();
          ret_code += id + CodeGen::emitOr(r1, left_var, right_var);
          ret_code += id + CodeGen::emitNe(ret_var, std::to_string(0), r1);
          break;
        }
        case LogicalOP::AND: {
          std::string r1 = env.NewTempVar();
          std::string r2 = env.NewTempVar();
          ret_code += id + CodeGen::emitNe(r1, std::to_string(0), left_var);
          ret_code += id + CodeGen::emitNe(r2, std::to_string(0), right_var);
          ret_code += id + CodeGen::emitAnd(ret_var, r1, r2);
          break;
        }
      }
      env.code << ret_code;
      return ret_var;
    }
};

