#include "CodeGenerator.h"
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

CodeGenerator::CodeGenerator(const vector<Quadruple>& quads)
    : quadruples(quads), constCounter(0) {
    collectVariablesAndConstants();
}

CodeGenerator::~CodeGenerator() {}

void CodeGenerator::collectVariablesAndConstants() {
    for (const auto& q : quadruples) {
        auto addOperand = [this](const string& op) {
            if (op.empty() || op == "_") return;

            bool isNumber = true;
            bool hasDot = false;
            for (char c : op) {
                if (c == '.') {
                    if (hasDot) {
                        isNumber = false;
                        break;
                    }
                    hasDot = true;
                }
                else if (!isdigit(c)) {
                    isNumber = false;
                    break;
                }
            }

            if (isNumber && !op.empty()) {
                constants.insert(op);
            }
            else if (op[0] == 't' && op.length() > 1 && isdigit(op[1])) {
                variables.insert(op);
            }
            else {
                variables.insert(op);
            }
            };

        addOperand(q.arg1);
        addOperand(q.arg2);
        addOperand(q.result);
    }
}

string CodeGenerator::getConstantName(const string& value) {
    if (constantNames.find(value) != constantNames.end()) {
        return constantNames[value];
    }

    string name = "const_" + to_string(++constCounter);
    string safeValue = value;
    for (char& c : safeValue) {
        if (c == '.') c = '_';
    }
    name = "const_" + safeValue;

    constantNames[value] = name;
    return name;
}

string CodeGenerator::getOperandName(const string& operand) {
    if (operand.empty() || operand == "_") return "";

    bool isNumber = true;
    bool hasDot = false;
    for (char c : operand) {
        if (c == '.') {
            if (hasDot) {
                isNumber = false;
                break;
            }
            hasDot = true;
        }
        else if (!isdigit(c)) {
            isNumber = false;
            break;
        }
    }

    if (isNumber && !operand.empty()) {
        return getConstantName(operand);
    }

    return operand;
}

string CodeGenerator::generateDataSection() {
    stringstream ss;

    ss << "; Data segment" << endl;
    ss << ".data" << endl;

    for (const auto& var : variables) {
        if (var[0] == 't' && var.length() > 1 && isdigit(var[1])) {
            ss << var << " dd ?" << endl;
        }
        else {
            ss << var << " dd ?" << endl;
        }
    }

    for (const auto& constVal : constants) {
        string name = getConstantName(constVal);
        ss << name << " dd " << constVal << endl;
    }

    ss << "temp_int dw ?" << endl;
    ss << "temp_float dd ?" << endl;
    ss << "buffer db 16 dup(?)" << endl;

    return ss.str();
}

string CodeGenerator::generateAssignment(const Quadruple& q) {
    stringstream ss;
    string src = getOperandName(q.arg1);
    string dst = getOperandName(q.result);

    if (src.empty() || dst.empty()) return "";

    ss << "    ; " << dst << " := " << src << endl;
    ss << "    fld " << src << endl;
    ss << "    fstp " << dst << endl;

    return ss.str();
}

string CodeGenerator::generateArithmetic(const Quadruple& q) {
    stringstream ss;
    string op = q.op;
    string arg1 = getOperandName(q.arg1);
    string arg2 = getOperandName(q.arg2);
    string result = getOperandName(q.result);

    ss << "    ; " << result << " := " << arg1 << " " << op << " " << arg2 << endl;
    ss << "    fld " << arg1 << endl;

    if (op == "+") {
        ss << "    fadd " << arg2 << endl;
    }
    else if (op == "-") {
        ss << "    fsub " << arg2 << endl;
    }
    else if (op == "*") {
        ss << "    fmul " << arg2 << endl;
    }
    else if (op == "/") {
        ss << "    fdiv " << arg2 << endl;
    }

    ss << "    fstp " << result << endl;

    return ss.str();
}

string CodeGenerator::generateComparison(const Quadruple& q) {
    stringstream ss;
    string arg1 = getOperandName(q.arg1);
    string arg2 = getOperandName(q.arg2);

    ss << "    ; compare " << arg1 << " " << q.op << " " << arg2 << endl;
    ss << "    fld " << arg1 << endl;
    ss << "    fcomp " << arg2 << endl;
    ss << "    fstsw ax" << endl;
    ss << "    sahf" << endl;

    return ss.str();
}

string CodeGenerator::generateRead(const Quadruple& q) {
    stringstream ss;
    string var = getOperandName(q.result);

    ss << "    ; read " << var << endl;
    ss << "    call ReadReal" << endl;
    ss << "    fstp " << var << endl;

    return ss.str();
}

string CodeGenerator::generateWrite(const Quadruple& q) {
    stringstream ss;
    string expr = getOperandName(q.arg1);

    ss << "    ; write " << expr << endl;
    ss << "    fld " << expr << endl;
    ss << "    call WriteReal" << endl;

    return ss.str();
}

string CodeGenerator::generateLabel(const Quadruple& q) {
    return q.result + ":\n";
}

string CodeGenerator::generateGoto(const Quadruple& q) {
    return "    jmp " + q.result + "\n";
}

string CodeGenerator::generateIfFalse(const Quadruple& q) {
    stringstream ss;
    string cond = getOperandName(q.arg1);
    string label = q.result;

    ss << "    ; if " << cond << " is false goto " << label << endl;
    ss << "    fldz" << endl;
    ss << "    fcomp " << cond << endl;
    ss << "    fstsw ax" << endl;
    ss << "    sahf" << endl;
    ss << "    jae " << label << endl;

    return ss.str();
}

string CodeGenerator::generateProlog() {
    stringstream ss;

    ss << "; Generated Assembly Code" << endl;
    ss << "; MASM-compatible syntax" << endl;
    ss << endl;
    ss << ".model small" << endl;
    ss << ".stack 100h" << endl;
    ss << endl;

    return ss.str();
}

string CodeGenerator::generateEpilog() {
    stringstream ss;

    ss << endl;
    ss << "    ; Exit program" << endl;
    ss << "    mov ax, 4C00h" << endl;
    ss << "    int 21h" << endl;
    ss << endl;

    return ss.str();
}

string CodeGenerator::generateIOProcedures() {
    stringstream ss;

    ss << "; Input/Output procedures" << endl;
    ss << endl;

    ss << "ReadReal proc" << endl;
    ss << "    push ax" << endl;
    ss << "    push bx" << endl;
    ss << "    push cx" << endl;
    ss << "    push dx" << endl;
    ss << "    push si" << endl;
    ss << "    push di" << endl;
    ss << "    " << endl;
    ss << "    mov ah, 0Ah" << endl;
    ss << "    mov dx, offset buffer" << endl;
    ss << "    mov byte ptr [buffer], 15" << endl;
    ss << "    int 21h" << endl;
    ss << "    " << endl;
    ss << "    finit" << endl;
    ss << "    fldz" << endl;
    ss << "    " << endl;
    ss << "    pop di" << endl;
    ss << "    pop si" << endl;
    ss << "    pop dx" << endl;
    ss << "    pop cx" << endl;
    ss << "    pop bx" << endl;
    ss << "    pop ax" << endl;
    ss << "    ret" << endl;
    ss << "ReadReal endp" << endl;
    ss << endl;

    ss << "WriteReal proc" << endl;
    ss << "    push ax" << endl;
    ss << "    push bx" << endl;
    ss << "    push cx" << endl;
    ss << "    push dx" << endl;
    ss << "    push si" << endl;
    ss << "    push di" << endl;
    ss << "    " << endl;
    ss << "    fstp temp_float" << endl;
    ss << "    " << endl;
    ss << "    mov ah, 02h" << endl;
    ss << "    mov dl, 0Dh" << endl;
    ss << "    int 21h" << endl;
    ss << "    mov dl, 0Ah" << endl;
    ss << "    int 21h" << endl;
    ss << "    " << endl;
    ss << "    pop di" << endl;
    ss << "    pop si" << endl;
    ss << "    pop dx" << endl;
    ss << "    pop cx" << endl;
    ss << "    pop bx" << endl;
    ss << "    pop ax" << endl;
    ss << "    ret" << endl;
    ss << "WriteReal endp" << endl;

    return ss.str();
}

string CodeGenerator::generateCodeSection() {
    stringstream ss;

    ss << ".code" << endl;
    ss << "start:" << endl;
    ss << "    mov ax, @data" << endl;
    ss << "    mov ds, ax" << endl;
    ss << "    finit" << endl;
    ss << endl;

    for (const auto& q : quadruples) {
        string code;

        if (q.op == ":=") {
            code = generateAssignment(q);
        }
        else if (q.op == "+" || q.op == "-" || q.op == "*" || q.op == "/") {
            code = generateArithmetic(q);
        }
        else if (q.op == "<" || q.op == ">" || q.op == "<=" || q.op == ">=" ||
            q.op == "=" || q.op == "<>") {
            code = generateComparison(q);
        }
        else if (q.op == "read") {
            code = generateRead(q);
        }
        else if (q.op == "write") {
            code = generateWrite(q);
        }
        else if (q.op == "label") {
            code = generateLabel(q);
        }
        else if (q.op == "goto") {
            code = generateGoto(q);
        }
        else if (q.op == "if_false") {
            code = generateIfFalse(q);
        }

        if (!code.empty()) {
            ss << code;
        }
    }

    ss << generateEpilog();
    ss << generateIOProcedures();
    ss << endl;
    ss << "end start" << endl;

    return ss.str();
}

string CodeGenerator::generate() {
    stringstream ss;

    ss << generateProlog();
    ss << generateDataSection();
    ss << endl;
    ss << generateCodeSection();

    return ss.str();
}

bool CodeGenerator::saveToFile(const string& filename) {
    string code = generate();

    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << " for writing" << endl;
        return false;
    }

    file << code;
    file.close();

    cout << "Assembly code saved to " << filename << endl;
    return true;
}