#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "SemanticAnalyzer.h"
#include <vector>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <cctype>
using namespace std;

class CodeGenerator {
private:
    vector<Quadruple> quadruples;
    set<string> variables;
    set<string> constants;
    map<string, string> constantNames;
    int constCounter;

    void collectVariablesAndConstants();
    string getConstantName(const string& value);
    string getOperandName(const string& operand);

    string generateDataSection();
    string generateCodeSection();
    string generateProlog();
    string generateEpilog();
    string generateIOProcedures();

    string generateAssignment(const Quadruple& q);
    string generateArithmetic(const Quadruple& q);
    string generateComparison(const Quadruple& q);
    string generateRead(const Quadruple& q);
    string generateWrite(const Quadruple& q);
    string generateLabel(const Quadruple& q);
    string generateGoto(const Quadruple& q);
    string generateIfFalse(const Quadruple& q);

public:
    CodeGenerator(const vector<Quadruple>& quads);
    ~CodeGenerator();

    string generate();
    bool saveToFile(const string& filename);
};

#endif