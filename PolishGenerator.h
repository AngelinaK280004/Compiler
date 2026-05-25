#ifndef POLISH_GENERATOR_H
#define POLISH_GENERATOR_H

#include "PrecedenceParser.h"
#include <vector>
#include <string>
#include <map>
using namespace std;

class PolishGenerator {
private:
    shared_ptr<ASTNode> ast;
    vector<string> polishCode;
    map<string, string> varMap;
    int tempCounter;
    int labelCounter;
    bool hasError;

    string newTemp();
    string newLabel();
    string generateExpression(shared_ptr<ASTNode> node);
    void generateStatement(shared_ptr<ASTNode> node);
    void generateAssignment(shared_ptr<ASTNode> node);
    void generateWhileLoop(shared_ptr<ASTNode> node);
    void generateRead(shared_ptr<ASTNode> node);
    void generateWrite(shared_ptr<ASTNode> node);
    void generateProgram(shared_ptr<ASTNode> node);

public:
    PolishGenerator(shared_ptr<ASTNode> root);
    ~PolishGenerator();

    bool generate();
    void printPolish() const;
    void savePolish(const string& filename) const;
    const vector<string>& getPolishCode() const;
    bool hasErrors() const;
};

#endif