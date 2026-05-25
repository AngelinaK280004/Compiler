#ifndef PYTHON_GENERATOR_H
#define PYTHON_GENERATOR_H

#include "PolishGenerator.h"
#include <vector>
#include <string>
#include <set>
#include <sstream>
#include <stack>
using namespace std;

class PythonGenerator {
private:
    vector<string> polishCode;
    set<string> variables;
    int indentLevel;
    int labelCounter;

    void collectVariables();
    string getIndent();
    string newLabel();
    string cleanExpression(const string& expr);  

public:
    PythonGenerator(const vector<string>& polish);
    ~PythonGenerator();

    string generate();
    bool saveToFile(const string& filename);
};

#endif