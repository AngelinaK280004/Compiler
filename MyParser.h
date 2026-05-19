#ifndef MYPARSER_H
#define MYPARSER_H

#include "Tokenizer.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>
using namespace std;

// Перечисление типов узлов AST
enum class NodeType {
    PROGRAM, BLOCK, VAR_DECL, VAR_LIST, ASSIGNMENT, WHILE_LOOP,
    READ_STATEMENT, WRITE_STATEMENT, STATEMENT_LIST, BINARY_OP,
    VARIABLE, NUMBER_LITERAL, EXPRESSION
};

// Узел AST
struct ASTNode {
    NodeType type;
    string value;
    vector<shared_ptr<ASTNode>> children;
    int line;

    ASTNode(NodeType t, const string& val = "", int l = 0);
    void addChild(shared_ptr<ASTNode> child);
};

// Класс синтаксического анализатора
class Parser {
private:
    vector<Token> tokens;
    int currentTokenIndex;
    Tokenizer& tokenizer;
    bool hasError;

    struct Symbol {
        string name;
        string type;
        int line;
    };
    vector<Symbol> symbolTable;

    // Вспомогательные методы
    Token getCurrentToken() const;
    void advance();
    bool match(TokenType type, int tableIndex = -1);
    bool matchKeyword(const string& keyword);
    bool matchOperator(const string& op);
    bool matchDelimiter(const string& delim);
    void expect(TokenType type, int tableIndex = -1);
    void expectKeyword(const string& keyword);
    void expectDelimiter(const string& delim);
    void error(const string& message);

    // Методы разбора по грамматике
    shared_ptr<ASTNode> parseProgram();
    shared_ptr<ASTNode> parseBlock();
    shared_ptr<ASTNode> parseVarDecl();
    shared_ptr<ASTNode> parseVarList();
    shared_ptr<ASTNode> parseStatementList();
    shared_ptr<ASTNode> parseStatement();
    shared_ptr<ASTNode> parseAssignment();
    shared_ptr<ASTNode> parseWhileLoop();
    shared_ptr<ASTNode> parseReadStatement();
    shared_ptr<ASTNode> parseWriteStatement();
    shared_ptr<ASTNode> parseExpression();
    shared_ptr<ASTNode> parseArithExpression();
    shared_ptr<ASTNode> parseTerm();
    shared_ptr<ASTNode> parseFactor();

public:
    Parser(Tokenizer& tok);
    ~Parser();

    shared_ptr<ASTNode> parse();
    void printAST(shared_ptr<ASTNode> node, int depth = 0);
    bool semanticAnalysis(shared_ptr<ASTNode> root);
    void printSymbolTable() const;
    bool hasErrors() const;
    const vector<Symbol>& getSymbolTable() const;
};

#endif