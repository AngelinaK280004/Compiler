#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "MyParser.h"   
#include <vector>
#include <string>
#include <map>
#include <fstream>
#include <iomanip>
using namespace std;

struct Quadruple {
    string op;
    string arg1;
    string arg2;
    string result;

    Quadruple(const string& o, const string& a1,
        const string& a2, const string& r)
        : op(o), arg1(a1), arg2(a2), result(r) {}

    string toString() const {
        return "(" + op + ", " + arg1 + ", " + arg2 + ", " + result + ")";
    }
};

class SemanticAnalyzer {
private:
    shared_ptr<ASTNode> ast;
    vector<Quadruple> quadruples;
    map<string, string> symbolTable;
    int tempCounter;
    int labelCounter;
    bool hasError;

    string newTemp();
    string newLabel();
    void addQuad(const string& op, const string& arg1,
        const string& arg2, const string& result);

    string generateExpression(shared_ptr<ASTNode> node);
    void generateStatement(shared_ptr<ASTNode> node);
    void generateAssignment(shared_ptr<ASTNode> node);
    void generateWhileLoop(shared_ptr<ASTNode> node);
    void generateRead(shared_ptr<ASTNode> node);
    void generateWrite(shared_ptr<ASTNode> node);
    void generateProgram(shared_ptr<ASTNode> node);

public:
    SemanticAnalyzer(shared_ptr<ASTNode> root);
    ~SemanticAnalyzer();

    bool analyze();
    void printQuadruples() const;
    void saveQuadruples(const string& filename) const;
    const vector<Quadruple>& getQuadruples() const { return quadruples; }
    bool hasErrors() const { return hasError; }
};

#endif