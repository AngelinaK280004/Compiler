#include "Tokenizer.h"
#include "PrecedenceParser.h"
#include "PolishGenerator.h"
#include "PythonGenerator.h"
#include <iostream>
#include <iomanip>
#include <chrono>
using namespace std;

void printHeader() {
    cout << "    PASCAL COMPILER" << endl;
    cout << endl;
}

void printStageHeader(const string& stage, int num) {
    cout << " STAGE " << num << ": " << left << setw(60) << stage << "" << endl;
}

int main(int argc, char* argv[]) {
    printHeader();

    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <source_file> [output_python] [output_polish]" << endl;
        cout << endl;
        cout << "Examples:" << endl;
        cout << "  " << argv[0] << " test_programs/test1_simple.txt" << endl;
        cout << "  " << argv[0] << " test_programs/test2_assignment.txt output.py polish.txt" << endl;
        return 1;
    }

    string sourceFile = argv[1];
    string pythonFile = (argc >= 3) ? argv[2] : "output.py";
    string polishFile = (argc >= 4) ? argv[3] : "polish.txt";

    cout << "Source file: " << sourceFile << endl;
    cout << "Output Python: " << pythonFile << endl;
    cout << "Output Polish: " << polishFile << endl;

    auto startTime = chrono::high_resolution_clock::now();
    bool hasError = false;

    printStageHeader("LEXICAL ANALYSIS", 1);

    Tokenizer tokenizer;
    if (!tokenizer.openFile(sourceFile)) {
        cerr << "Failed to open source file" << endl;
        return 1;
    }

    vector<Token> allTokens;
    Token token;
    do {
        token = tokenizer.getNextToken();
        allTokens.push_back(token);
    } while (token.type != TokenType::END);

    tokenizer.printTables();
    tokenizer.printTokens(allTokens);
    cout << "\n[OK] Lexical analysis completed successfully" << endl;
    tokenizer.closeFile();

    printStageHeader("SYNTAX ANALYSIS", 2);

    Tokenizer parserTokenizer;
    if (!parserTokenizer.openFile(sourceFile)) {
        cerr << "Failed to open source file" << endl;
        return 1;
    }

    PrecedenceParser parser(parserTokenizer);
    if (!parser.parse()) {
        cerr << "[FAIL] Syntax analysis failed" << endl;
        parser.printErrors();
        hasError = true;
    }
    else {
        cout << "\n[OK] Syntax analysis completed successfully" << endl;
        parser.printSymbolTable();
        parser.printASTree();
    }
    parserTokenizer.closeFile();

    printStageHeader("SEMANTIC ANALYSIS ", 3);

    if (!hasError) {
        PolishGenerator polishGen(parser.getAST());
        if (polishGen.generate()) {
            cout << "[OK] Polish notation generated successfully" << endl;
            polishGen.printPolish();
            polishGen.savePolish(polishFile);
        }
        else {
            cerr << "[FAIL] Failed to generate Polish notation" << endl;
            hasError = true;
        }
    }
    else {
        cerr << "[SKIP] Skipped due to previous errors" << endl;
    }

    printStageHeader("PYTHON CODE GENERATION", 4);

    if (!hasError) {
        PolishGenerator polishGen(parser.getAST());
        polishGen.generate();

        PythonGenerator pythonGen(polishGen.getPolishCode());
        if (pythonGen.saveToFile(pythonFile)) {
            cout << "[OK] Python code generated successfully" << endl;
            cout << "\nTo run the generated Python code:" << endl;
            cout << "  python " << pythonFile << endl;
        }
        else {
            cerr << "[FAIL] Failed to generate Python code" << endl;
            hasError = true;
        }
    }
    else {
        cerr << "[SKIP] Skipped due to previous errors" << endl;
    }

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);

    if (hasError) {
        cout << "    COMPILATION FAILED" << endl;
    }
    else {
        cout << "    COMPILATION SUCCESSFUL" << endl;
        cout << endl;
        cout << "Generated files:" << endl;
        cout << "  - " << pythonFile << " (Python code)" << endl;
        cout << "  - " << polishFile << " (Polish notation)" << endl;
    }
    cout << "\nCompilation time: " << duration.count() << " ms" << endl;
    cout << endl;

    cout << "Press Enter to exit..." << endl;
    cin.get();

    return hasError ? 1 : 0;
}