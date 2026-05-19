#include "Tokenizer.h"
#include "MyParser.h"
#include "SemanticAnalyzer.h"
#include "CodeGenerator.h"
#include <iostream>
#include <memory>
using namespace std;

void printUsage(const char* programName) {
    cout << "Pascal Compiler - Console Application" << endl;
    cout << "=====================================" << endl;
    cout << endl;
    cout << "Usage:" << endl;
    cout << "  " << programName << " <source_file> [output_asm] [output_tetrads]" << endl;
    cout << endl;
    cout << "Examples:" << endl;
    cout << "  " << programName << " test.txt" << endl;
    cout << "  " << programName << " test.txt output.asm tetrads.txt" << endl;
    cout << endl;
    cout << "The program performs:" << endl;
    cout << "  1. Lexical analysis" << endl;
    cout << "  2. Syntax analysis (recursive descent)" << endl;
    cout << "  3. Semantic analysis & quadruple generation" << endl;
    cout << "  4. Object code generation (MASM format)" << endl;
}

int main(int argc, char* argv[]) {
    cout << "========================================" << endl;
    cout << "    PASCAL COMPILER (Console Application)" << endl;
    cout << "========================================" << endl;
    cout << endl;

    if (argc < 2) {
        printUsage(argv[0]);
        cout << endl;
        cout << "Press Enter to exit..." << endl;
        cin.get();
        return 1;
    }

    string sourceFile = argv[1];
    string asmFile = (argc >= 3) ? argv[2] : "output.asm";
    string tetradsFile = (argc >= 4) ? argv[3] : "tetrads.txt";

    cout << "Source file: " << sourceFile << endl;
    cout << "Output ASM: " << asmFile << endl;
    cout << "Output tetrads: " << tetradsFile << endl;
    cout << endl;

    // STAGE 1: LEXICAL ANALYSIS
    cout << "=== STAGE 1: LEXICAL ANALYSIS ===" << endl;
    Tokenizer tokenizer;
    if (!tokenizer.openFile(sourceFile)) {
        cerr << "Failed to open source file" << endl;
        cout << "Press Enter to exit..." << endl;
        cin.get();
        return 1;
    }
    cout << "Lexical analysis completed successfully" << endl;

    // STAGE 2: SYNTAX ANALYSIS
    cout << "\n=== STAGE 2: SYNTAX ANALYSIS (Recursive Descent) ===" << endl;
    Parser parser(tokenizer);
    auto ast = parser.parse();

    if (parser.hasErrors()) {
        cerr << "Syntax analysis failed" << endl;
        cout << "Press Enter to exit..." << endl;
        cin.get();
        return 1;
    }
    cout << "Syntax analysis successful" << endl;

    // STAGE 3: SEMANTIC ANALYSIS
    cout << "\n=== STAGE 3: SEMANTIC ANALYSIS ===" << endl;

    if (!parser.semanticAnalysis(ast)) {
        cerr << "Semantic analysis failed" << endl;
        cout << "Press Enter to exit..." << endl;
        cin.get();
        return 1;
    }
    parser.printSymbolTable();

    SemanticAnalyzer semanticAnalyzer(ast);
    if (!semanticAnalyzer.analyze()) {
        cerr << "Failed to generate quadruples" << endl;
        cout << "Press Enter to exit..." << endl;
        cin.get();
        return 1;
    }

    semanticAnalyzer.printQuadruples();
    semanticAnalyzer.saveQuadruples(tetradsFile);

    // STAGE 4: CODE GENERATION
    cout << "\n=== STAGE 4: OBJECT CODE GENERATION ===" << endl;
    CodeGenerator codeGenerator(semanticAnalyzer.getQuadruples());

    if (codeGenerator.saveToFile(asmFile)) {
        cout << "\n========================================" << endl;
        cout << "    COMPILATION SUCCESSFUL!" << endl;
        cout << "========================================" << endl;
        cout << "Generated files:" << endl;
        cout << "  - " << asmFile << " (assembly code)" << endl;
        cout << "  - " << tetradsFile << " (intermediate code)" << endl;
        cout << endl;
        cout << "To create executable using Open Watcom:" << endl;
        cout << "  wasm " << asmFile << endl;
        cout << "  wlink file " << asmFile.substr(0, asmFile.find_last_of('.')) << ".obj" << endl;
    }
    else {
        cerr << "Failed to generate assembly code" << endl;
    }

    cout << endl;
    cout << "Press Enter to exit..." << endl;
    cin.get();

    return 0;
}