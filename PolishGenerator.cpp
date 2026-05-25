#include "PolishGenerator.h"
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

PolishGenerator::PolishGenerator(shared_ptr<ASTNode> root)
    : ast(root), tempCounter(0), labelCounter(0), hasError(false) {}

PolishGenerator::~PolishGenerator() {}

string PolishGenerator::newTemp() {
    return "t" + to_string(++tempCounter);
}

string PolishGenerator::newLabel() {
    return "L" + to_string(++labelCounter);
}

string PolishGenerator::generateExpression(shared_ptr<ASTNode> node) {
    if (!node) return "";

    switch (node->type) {
    case NodeType::VARIABLE:
        return node->value;
    case NodeType::NUMBER_LITERAL:
        return node->value;
    case NodeType::BINARY_OP: {
        if (node->children.size() < 2) return "";
        string left = generateExpression(node->children[0]);
        string right = generateExpression(node->children[1]);
        string temp = newTemp();
        polishCode.push_back(left);
        polishCode.push_back(right);
        polishCode.push_back(node->value);
        polishCode.push_back(temp + " := " + left + " " + node->value + " " + right);
        return temp;
    }
    case NodeType::EXPRESSION:
        if (!node->children.empty())
            return generateExpression(node->children[0]);
        return "";
    default:
        return "";
    }
}

void PolishGenerator::generateAssignment(shared_ptr<ASTNode> node) {
    if (!node || node->children.size() < 2) return;
    string var = node->children[0]->value;
    string expr = generateExpression(node->children[1]);
    polishCode.push_back(expr);
    polishCode.push_back(var + " := " + expr);
}

void PolishGenerator::generateWhileLoop(shared_ptr<ASTNode> node) {
    if (!node || node->children.size() < 2) return;
    string startLabel = newLabel();
    string endLabel = newLabel();

    polishCode.push_back(startLabel + ":");
    string cond = generateExpression(node->children[0]);
    polishCode.push_back(cond);
    polishCode.push_back("if_false goto " + endLabel);
    generateStatement(node->children[1]);
    polishCode.push_back("goto " + startLabel);
    polishCode.push_back(endLabel + ":");
}

void PolishGenerator::generateRead(shared_ptr<ASTNode> node) {
    if (!node || node->children.empty()) return;
    string var = node->children[0]->value;
    polishCode.push_back("read " + var);
}

void PolishGenerator::generateWrite(shared_ptr<ASTNode> node) {
    if (!node || node->children.empty()) return;
    string expr = generateExpression(node->children[0]);
    polishCode.push_back(expr);
    polishCode.push_back("write");
}

void PolishGenerator::generateStatement(shared_ptr<ASTNode> node) {
    if (!node) return;

    switch (node->type) {
    case NodeType::ASSIGNMENT:
        generateAssignment(node);
        break;
    case NodeType::WHILE_LOOP:
        generateWhileLoop(node);
        break;
    case NodeType::READ_STATEMENT:
        generateRead(node);
        break;
    case NodeType::WRITE_STATEMENT:
        generateWrite(node);
        break;
    case NodeType::STATEMENT_LIST:
        for (const auto& child : node->children) {
            generateStatement(child);
        }
        break;
    default:
        break;
    }
}

void PolishGenerator::generateProgram(shared_ptr<ASTNode> node) {
    if (!node) return;
    for (const auto& child : node->children) {
        if (child->type == NodeType::BLOCK) {
            for (const auto& blockChild : child->children) {
                if (blockChild->type == NodeType::STATEMENT_LIST) {
                    generateStatement(blockChild);
                }
            }
        }
    }
}

bool PolishGenerator::generate() {
    if (!ast) {
        hasError = true;
        return false;
    }
    generateProgram(ast);
    return !hasError;
}

void PolishGenerator::printPolish() const {
    cout << "\n=== POLISH NOTATION ===" << endl;
    for (size_t i = 0; i < polishCode.size(); ++i) {
        cout << (i + 1) << ": " << polishCode[i] << endl;
    }
}

void PolishGenerator::savePolish(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << " for writing" << endl;
        return;
    }
    file << "=== REVERSE POLISH NOTATION ===" << endl;
    for (size_t i = 0; i < polishCode.size(); ++i) {
        file << (i + 1) << ": " << polishCode[i] << endl;
    }
    file.close();
    cout << "Polish notation saved to " << filename << endl;
}

const vector<string>& PolishGenerator::getPolishCode() const {
    return polishCode;
}

bool PolishGenerator::hasErrors() const {
    return hasError;
}