#include "SemanticAnalyzer.h"
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

SemanticAnalyzer::SemanticAnalyzer(shared_ptr<ASTNode> root)
    : ast(root), tempCounter(0), labelCounter(0), hasError(false) {
}

SemanticAnalyzer::~SemanticAnalyzer() {}

string SemanticAnalyzer::newTemp() {
    return "t" + to_string(++tempCounter);
}

string SemanticAnalyzer::newLabel() {
    return "L" + to_string(++labelCounter);
}

void SemanticAnalyzer::addQuad(const string& op, const string& arg1,
    const string& arg2, const string& result) {
    quadruples.push_back(Quadruple(op, arg1, arg2, result));
}

string SemanticAnalyzer::generateExpression(shared_ptr<ASTNode> node) {
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
        addQuad(node->value, left, right, temp);
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

void SemanticAnalyzer::generateAssignment(shared_ptr<ASTNode> node) {
    if (!node || node->children.size() < 2) return;
    // left child: variable
    string var = node->children[0]->value;
    // right child: expression
    string expr = generateExpression(node->children[1]);
    addQuad(":=", expr, "_", var);
}

void SemanticAnalyzer::generateWhileLoop(shared_ptr<ASTNode> node) {
    if (!node || node->children.size() < 2) return;
    string startLabel = newLabel();
    string endLabel = newLabel();

    addQuad("label", "_", "_", startLabel);
    string cond = generateExpression(node->children[0]);
    addQuad("if_false", cond, "_", endLabel);
    generateStatement(node->children[1]);
    addQuad("goto", "_", "_", startLabel);
    addQuad("label", "_", "_", endLabel);
}

void SemanticAnalyzer::generateRead(shared_ptr<ASTNode> node) {
    if (!node || node->children.empty()) return;
    string var = node->children[0]->value;
    addQuad("read", "_", "_", var);
}

void SemanticAnalyzer::generateWrite(shared_ptr<ASTNode> node) {
    if (!node || node->children.empty()) return;
    string expr = generateExpression(node->children[0]);
    addQuad("write", expr, "_", "_");
}

void SemanticAnalyzer::generateStatement(shared_ptr<ASTNode> node) {
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
        // »гнорируем другие типы узлов (например, VAR_DECL, BLOCK)
        break;
    }
}

void SemanticAnalyzer::generateProgram(shared_ptr<ASTNode> node) {
    if (!node) return;
    // ”зел PROGRAM: первый дочерний элемент Ч им€ программы (мы его не используем),
    // второй дочерний Ч BLOCK.
    for (const auto& child : node->children) {
        if (child->type == NodeType::BLOCK) {
            // ¬ блоке могут быть VAR_DECL и STATEMENT_LIST
            for (const auto& blockChild : child->children) {
                if (blockChild->type == NodeType::STATEMENT_LIST) {
                    generateStatement(blockChild);
                }
                // VAR_DECL игнорируем (тетрады дл€ объ€влени€ не нужны)
            }
        }
    }
}

bool SemanticAnalyzer::analyze() {
    if (!ast) {
        hasError = true;
        return false;
    }
    generateProgram(ast);
    return !hasError;
}

void SemanticAnalyzer::printQuadruples() const {
    cout << "\n=== QUADRUPLES (INTERMEDIATE CODE) ===" << endl;
    cout << left << setw(6) << "є"
        << setw(12) << "op"
        << setw(12) << "arg1"
        << setw(12) << "arg2"
        << setw(12) << "result" << endl;
    cout << string(54, '-') << endl;

    for (size_t i = 0; i < quadruples.size(); ++i) {
        cout << left << setw(6) << (i + 1)
            << setw(12) << quadruples[i].op
            << setw(12) << quadruples[i].arg1
            << setw(12) << quadruples[i].arg2
            << setw(12) << quadruples[i].result << endl;
    }
}

void SemanticAnalyzer::saveQuadruples(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Cannot open file " << filename << " for writing" << endl;
        return;
    }
    for (size_t i = 0; i < quadruples.size(); ++i) {
        file << (i + 1) << ": " << quadruples[i].toString() << endl;
    }
    file.close();
    cout << "Quadruples saved to " << filename << endl;
}
