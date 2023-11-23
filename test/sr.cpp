#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

enum OperandType {
    REGISTER,
    IMMEDIATE,
    LABEL,
    // 其他类型
};

struct Operand {
    OperandType type;
    std::string value;

    Operand(OperandType t, const std::string& v) : type(t), value(v) {}

    static Operand Parse(const std::string& operandStr) {
        // 简化解析，实际情况可能需要更复杂的逻辑
        if (operandStr.find_first_of("%") == 0) {
            // 如果操作数以 '%' 开头，则为寄存器
            return Operand(REGISTER, operandStr);
        } else if (operandStr.find_first_of("@") == 0) {
            // 如果操作数以 '@' 开头，则为标签
            return Operand(LABEL, operandStr);
        } else {
            // 其他情况简单地认为是立即数
            return Operand(IMMEDIATE, operandStr);
        }
    }

    std::string Dump() const {
        return value;  // 简化，实际情况可能需要根据类型和值输出不同格式
    }
};

struct Instruction {
    std::string opcode;
    std::vector<Operand> operands;

    static Instruction Parse(const std::string& instructionStr) {
        Instruction instr;
        std::istringstream iss(instructionStr);
        iss >> instr.opcode;

        std::string operandStr;
        while (iss >> operandStr) {
            instr.operands.push_back(Operand::Parse(operandStr));
        }

        return instr;
    }

    std::string Dump() const {
        std::string result = opcode;
        for (const auto& operand : operands) {
            result += " " + operand.Dump();
        }
        return result;
    }
};

struct BasicBlock {
    std::vector<Instruction> instructions;

    void print() const {
        for (const auto& instr : instructions) {
            std::cout << instr.Dump() << std::endl;
        }
    }

    static BasicBlock Parse(const std::vector<std::string>& irLines) {
        BasicBlock block;
        for (const auto& line : irLines) {
            block.instructions.push_back(Instruction::Parse(line));
        }
        return block;
    }
};

int main() {
    // 示例 LLVM IR 代码
    std::vector<std::string> llvmIR = {
        "entry:",
        "  %1 = add i32 2, 3",
        "  %result = mul i32 %1, 4",
        "  br label %next",
        "next:",
        "  %2 = add i32 %result, 1",
        "  ret i32 %2"
    };

    // 解析 LLVM IR 并构造数据结构
    BasicBlock block = BasicBlock::Parse(llvmIR);

    // 打印结果
    block.print();

    return 0;
}
