#ifndef PRECEDENCE_PARSER_H
#define PRECEDENCE_PARSER_H

#include "Tokenizer.h"
#include <stack>
#include <vector>
#include <map>
#include <memory>
#include <functional>
using namespace std;

enum class NodeType {
    PROGRAM, BLOCK, VAR_DECL, VAR_LIST, ASSIGNMENT, WHILE_LOOP,
    READ_STATEMENT, WRITE_STATEMENT, STATEMENT_LIST, BINARY_OP,
    VARIABLE, NUMBER_LITERAL, EXPRESSION
};

struct ASTNode {
    NodeType type;
    string value;
    vector<shared_ptr<ASTNode>> children;
    int line;

    ASTNode(NodeType t, const string& val = "", int l = 0)
        : type(t), value(val), line(l) {}

    void addChild(shared_ptr<ASTNode> child) {
        children.push_back(child);
    }
};

class PrecedenceParser {
private:
    vector<Token> tokens;
    int currentPos;
    map<pair<string, string>, char> precedenceTable;
    bool hasError;
    vector<string> errors;

    struct Symbol {
        string name;
        string type;
        int line;
    };
    vector<Symbol> symbolTable;
    shared_ptr<ASTNode> root;

    void buildPrecedenceTable();
    char getPrecedence(const string& left, const string& right);
    void error(const string& message, const Token& token);

    // Методы разбора
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

    void printAST(shared_ptr<ASTNode> node, int depth) const;
    void buildSymbolTable();

public:
    PrecedenceParser(Tokenizer& tokenizer);
    ~PrecedenceParser();

    bool parse();
    void printErrors() const;
    void printSymbolTable() const;
    void printASTree() const;

    shared_ptr<ASTNode> getAST() const { return root; }
    bool hasErrors() const { return hasError; }
    const vector<Symbol>& getSymbolTable() const { return symbolTable; }
};

#endif