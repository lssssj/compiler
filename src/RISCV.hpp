#pragma once

#include <assert.h>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <regex>
#include <string>
#include <unordered_map>
#include "ast.hpp"
#include "koopa.h"

// 函数声明略
// ...

static std::string reg_names[16] = {"x0", "t0", "t1", "t2", "t3", "t4", "t5", "t6", 
                           "a0", "a1","a2","a3","a4","a5","a6","a7"};
struct Reg {
  int offset;
  bool stack;

  friend std::ostream& operator<<(std::ostream& os, const Reg& r) {
      os << "Reg{offset=" << r.offset << ", stack=" << r.stack << "}" << std::endl;
      return os;
  }
};

class RISCVEnvironemt {
  private:
    int reg_state[16] = {0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,-1, -1, -1};
  public:
    std::ostringstream code;
    std::unordered_map<koopa_raw_value_t, Reg> value_map;

    int zeroReg = 0;
    int retReg = 8;
    int stack_top = 0;

    Reg FindReg() {
      for (int i = 1; i < 16; i++) {
        if (reg_state[i] == -1) {
          SetRegBusy(i);
          return Reg{i};
        }
      }
      return Reg {-1};
    }

    Reg allocStack() {
      Reg r{.offset = stack_top, .stack = true};
      stack_top += 4;
      return r;
    }

    void SetRegBusy(int i) {
      reg_state[i] = 1;
    }

    void SetRegFree(int i) {
      reg_state[i] = -1;
    }

    void alloc(const koopa_raw_type_t &ty) {
      int size = cal_size(ty);
      stack_top += size;
    }

    int cal_size(const koopa_raw_type_t &ty) {
      if (ty->tag == KOOPA_RTT_UNIT) {
        return 0;
      }
      if (ty->tag == KOOPA_RTT_ARRAY) {
        return ty->data.array.len * cal_size(ty->data.array.base);
      }
      return 4;
    }
};

class RISCVCodeGen {
  public:
    static std::string emitMv(int destReg, int srcReg) {
      return "\tmv " + reg_names[destReg] + ", " + reg_names[srcReg] + "\n";
    }

    static std::string emitRet() {
      return "\tret\n";
    }

    static std::string emitStackAddi(int offset) {
      return "\taddi sp, sp, " + std::to_string(offset) + "\n";
    }

    static std::string emitLi(int destReg, int num) {
      return "\tli " + reg_names[destReg] + ", " + std::to_string(num) + "\n"; 
    }

    static std::string emitSeqz(int destReg, int opReg) {
      return "\tseqz " + reg_names[destReg] + ", " + reg_names[opReg] + "\n";
    }

    static std::string emitSnez(int destReg, int opReg) {
      return "\tsnez " + reg_names[destReg] + ", " + reg_names[opReg] + "\n";
    }

    static std::string emitSlt(int destReg, int opReg1, int opReg2) {
      return "\tslt " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitSgt(int destReg, int opReg1, int opReg2) {
      return "\tsgt " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitSub(int destReg, int opReg1, int opReg2) {
      return "\tsub " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitAdd(int destReg, int opReg1, int opReg2) {
      return "\tadd " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitMul(int destReg, int opReg1, int opReg2) {
      return "\tmul " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitDiv(int destReg, int opReg1, int opReg2) {
      return "\tdiv " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitRem(int destReg, int opReg1, int opReg2) {
      return "\trem " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitAnd(int destReg, int opReg1, int opReg2) {
      return "\tand " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitOr(int destReg, int opReg1, int opReg2) {
      return "\tor " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitXor(int destReg, int opReg1, int opReg2) {
      return "\txor " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitSll(int destReg, int opReg1, int opReg2) {
      return "\tsll " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitSrl(int destReg, int opReg1, int opReg2) {
      return "\tsrl " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitSra(int destReg, int opReg1, int opReg2) {
      return "\tsra " + reg_names[destReg] + ", " + reg_names[opReg1] + ", " + reg_names[opReg2] + "\n";
    }

    static std::string emitSw(std::string src, std::string dest) {
      return "\tsw " + src + ", " + dest + "\n";
    }

    static std::string emitLw(std::string src, std::string dest) {
      return "\tlw " + src + ", " + dest + "\n";
    }
};

void Visit(RISCVEnvironemt &env, const koopa_raw_program_t &program);
// 访问所有全局变量
void Visit(RISCVEnvironemt &env, const koopa_raw_slice_t &slice);
// 访问函数
void Visit(RISCVEnvironemt &env, const koopa_raw_function_t &func);
// 访问基本块
void Visit(RISCVEnvironemt &env, const koopa_raw_basic_block_t &bb);
// 访问指令
Reg Visit(RISCVEnvironemt &env, const koopa_raw_value_t &value);

Reg Visit(RISCVEnvironemt &env, const koopa_raw_return_t &ret);
Reg Visit(RISCVEnvironemt &envt, const koopa_raw_integer_t &val);
Reg Visit(RISCVEnvironemt &env, const koopa_raw_binary_t &val);
Reg Visit(RISCVEnvironemt &env, const koopa_raw_load_t &val);
Reg Visit(RISCVEnvironemt &env, const koopa_raw_store_t &val);

// 访问 raw program
void Visit(RISCVEnvironemt &env, const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  Visit(env, program.values);
  // 访问所有函数
  env.code << "\t.text\n";
  Visit(env, program.funcs);
}

// 访问 raw slice
void Visit(RISCVEnvironemt &env, const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(env, reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(env, reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(env, reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问函数
void Visit(RISCVEnvironemt &env, const koopa_raw_function_t &func) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有基本块
  env.code << " \t.global " << (func->name + 1) << "\n";
  env.code << (func->name + 1) << ":\n";
  assert(env.stack_top == 0);
  int stack_size = 0;
  for (size_t i = 0; i < func->bbs.len; i++) {
    auto ptr = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
    for (size_t j = 0; j < ptr->insts.len; j++) {
      auto inst = reinterpret_cast<koopa_raw_value_t>(ptr->insts.buffer[j]);
      stack_size += env.cal_size(inst->ty);
    }
  }
  if (stack_size != 0) {
    env.code << RISCVCodeGen::emitStackAddi(-stack_size);
  }
  Visit(env, func->bbs);
}



// 访问基本块
void Visit(RISCVEnvironemt &env, const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(env, bb->insts);
}

// 访问指令
Reg Visit(RISCVEnvironemt &env, const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  if (env.value_map.count(value)) {
    return env.value_map[value];
  }
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN: {
      // 访问 return 指令
      // ret没有返回值 0
      return Visit(env, kind.data.ret);
    }
      
    case KOOPA_RVT_INTEGER: {
      // 访问 integer 指令
      // 有返回值 4
      Reg r = Visit(env, kind.data.integer);
      env.value_map[value] = r;
      return r;
    }
    case KOOPA_RVT_BINARY:  {
      // 访问 binary op 
      // 有返回值 4
      Reg r = Visit(env, kind.data.binary);
      env.value_map[value] = r;
      return r;
    }
    case KOOPA_RVT_ALLOC: {
      // alloc有返回值
      Reg r = {.offset = env.stack_top, .stack = true};
      env.alloc(value->ty);
      env.value_map[value] = r;
      return r;
    }
    case KOOPA_RVT_LOAD: {
      // load有返回值
      Reg r = Visit(env, kind.data.load);
      env.value_map[value] = r;
      return r;
    }
    case KOOPA_RVT_STORE: {
      // store没有返回值
      Reg v = Visit(env, kind.data.store);
      env.value_map[value] = v;
      return v;
    }
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...
Reg Visit(RISCVEnvironemt &env, const koopa_raw_return_t &ret) {
  Reg r = Visit(env, ret.value);
  if (r.stack) {
    env.code << RISCVCodeGen::emitLw(reg_names[env.retReg] , std::to_string(r.offset) + "(sp)");
  } else {
    env.code << RISCVCodeGen::emitMv(env.retReg, r.offset);
  }
  if (env.stack_top != 0) {
    env.code << RISCVCodeGen::emitStackAddi(env.stack_top);
    env.stack_top = 0;
  }
  env.code << RISCVCodeGen::emitRet(); 
  return Reg {-1};
}

Reg Visit(RISCVEnvironemt &env, const koopa_raw_integer_t &val) {
  if (val.value == 0) {
    return Reg{env.zeroReg};
  }
  Reg reg = env.FindReg();
  env.code << RISCVCodeGen::emitLi(reg.offset, val.value);
  return reg;
}

Reg Visit(RISCVEnvironemt &env, const koopa_raw_binary_t &val) {
  Reg lhs = Visit(env, val.lhs);
  Reg rhs = Visit(env, val.rhs);
  if (lhs.stack) {
    Reg temp = env.FindReg();
    env.code << RISCVCodeGen::emitLw(reg_names[temp.offset], std::to_string(lhs.offset) + "(sp)");
    lhs = temp;
  }
  if (rhs.stack) {
    Reg temp = env.FindReg();
    env.code << RISCVCodeGen::emitLw(reg_names[temp.offset], std::to_string(rhs.offset) + "(sp)");
    rhs = temp;
  }
  Reg ret = env.FindReg();
  switch (val.op) {
    case KOOPA_RBO_EQ:
      env.code << RISCVCodeGen::emitXor(ret.offset, lhs.offset, rhs.offset);
      env.code << RISCVCodeGen::emitSeqz(ret.offset, ret.offset);
      break;
    case KOOPA_RBO_NOT_EQ:
      env.code << RISCVCodeGen::emitXor(ret.offset, lhs.offset, rhs.offset);
      env.code << RISCVCodeGen::emitSnez(ret.offset, ret.offset);
      break;
    case KOOPA_RBO_GT:
      env.code << RISCVCodeGen::emitSgt(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_LT:
      env.code << RISCVCodeGen::emitSlt(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_GE:
      env.code << RISCVCodeGen::emitSub(ret.offset, lhs.offset, rhs.offset);
      env.code << RISCVCodeGen::emitSlt(ret.offset, env.zeroReg, ret.offset);
      break;
    case KOOPA_RBO_LE:
      env.code << RISCVCodeGen::emitSub(ret.offset, lhs.offset, rhs.offset);
      env.code << RISCVCodeGen::emitSgt(ret.offset, env.zeroReg, ret.offset);
      break;
    case KOOPA_RBO_ADD:
      env.code << RISCVCodeGen::emitAdd(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_SUB:
      env.code << RISCVCodeGen::emitSub(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_MUL:
      env.code << RISCVCodeGen::emitMul(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_DIV:
      env.code << RISCVCodeGen::emitDiv(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_MOD:
      env.code << RISCVCodeGen::emitRem(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_AND: 
      env.code << RISCVCodeGen::emitAnd(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_OR:
      env.code << RISCVCodeGen::emitOr(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_XOR:
      env.code << RISCVCodeGen::emitXor(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_SHL:
      env.code << RISCVCodeGen::emitSll(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_SHR:
      env.code << RISCVCodeGen::emitSrl(ret.offset, lhs.offset, rhs.offset);
      break;
    case KOOPA_RBO_SAR:  
      env.code << RISCVCodeGen::emitSra(ret.offset, lhs.offset, rhs.offset);
      break;
  }
  env.SetRegFree(lhs.offset);
  env.SetRegFree(rhs.offset);
  Reg r = env.allocStack();
  env.code << RISCVCodeGen::emitSw(reg_names[ret.offset], std::to_string(r.offset) + "(sp)");
  env.SetRegFree(ret.offset);
  return r;
}

Reg Visit(RISCVEnvironemt &env, const koopa_raw_load_t &val) {
  Reg r = Visit(env, val.src);
  // Reg dest = env.FindReg();
  // if (dest.offset == -1) {
  //   dest = env.allocStack();
  // }
  Reg dest = env.allocStack();
  if (!r.stack && !dest.stack) {
    env.code << RISCVCodeGen::emitMv(dest.offset, r.offset);
  } else if (!r.stack && dest.stack) {
    env.code << RISCVCodeGen::emitSw(reg_names[r.offset], std::to_string(dest.offset) + "(sp)");
  } else if (r.stack && !dest.stack) {
    env.code << RISCVCodeGen::emitLw(reg_names[dest.offset], std::to_string(r.offset) + "(sp)");
  } else {
    Reg temp = env.FindReg();
    env.code << RISCVCodeGen::emitLw(reg_names[temp.offset], std::to_string(r.offset) + "(sp)");
    env.code << RISCVCodeGen::emitSw(reg_names[temp.offset], std::to_string(dest.offset) + "(sp)");
    env.SetRegFree(temp.offset);
  }
  return dest;
}

Reg Visit(RISCVEnvironemt &env, const koopa_raw_store_t &val) {
  Reg dest = Visit(env, val.dest);
  Reg value = Visit(env, val.value);
  if (value.stack) {
    Reg temp = env.FindReg();
    env.code << RISCVCodeGen::emitLw(reg_names[temp.offset], std::to_string(value.offset) + "(sp)");
    env.code << RISCVCodeGen::emitSw(reg_names[temp.offset], std::to_string(dest.offset) + "(sp)");
    env.SetRegFree(temp.offset);
  } else {
    env.code << RISCVCodeGen::emitSw(reg_names[value.offset], std::to_string(dest.offset) + "(sp)");
    env.SetRegFree(value.offset);
  }
  return value;
}
