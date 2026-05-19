#include "Parser.h"
#include <iostream>
#include <iomanip>
using namespace std;

Parser::Parser(Tokenizer& tok)
    : tokenizer(tok), currentTokenIndex(0), hasError(false) {

    Token token;
    do {
        token = tokenizer.getNextToken();
        tokens.push_back(token);
    } while (token.type != TokenType::END);
}

Parser::~Parser() {}

Token Parser::getCurrentToken() const {
    if (currentTokenIndex < (int)tokens.size()) {
        return tokens[currentTokenIndex];
    }
    Token endToken;
    endToken.type = TokenType::END;
    return endToken;
}

void Parser::advance() {
    if (currentTokenIndex < (int)tokens.size()) {
        currentTokenIndex++;
    }
}

bool Parser::match(TokenType type, int tableIndex) {
    Token current = getCurrentToken();
    if (current.type == type) {
        if (tableIndex == -1 || current.tableIndex == tableIndex) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::matchKeyword(const string& keyword) {
    Token current = getCurrentToken();
    if (current.type == TokenType::KEYWORD) {
        const auto& keywords = tokenizer.getKeywords();
        if (current.tableIndex >= 0 && current.tableIndex < (int)keywords.size()) {
            if (keywords[current.tableIndex] == keyword) {
                advance();
                return true;
            }
        }
    }
    return false;
}

bool Parser::matchOperator(const string& op) {
    Token current = getCurrentToken();
    if (current.type == TokenType::OPERATOR) {
        const auto& operators = tokenizer.getOperators();
        if (current.tableIndex >= 0 && current.tableIndex < (int)operators.size()) {
            if (operators[current.tableIndex] == op) {
                advance();
                return true;
            }
        }
    }
    return false;
}

bool Parser::matchDelimiter(const string& delim) {
    Token current = getCurrentToken();
    if (current.type == TokenType::DELIMITER) {
        const auto& delimiters = tokenizer.getDelimiters();
        if (current.tableIndex >= 0 && current.tableIndex < (int)delimiters.size()) {
            if (delimiters[current.tableIndex] == delim) {
                advance();
                return true;
            }
        }
    }
    return false;
}

void Parser::expect(TokenType type, int tableIndex) {
    if (!match(type, tableIndex)) {
        error("Expected token type " + to_string((int)type));
    }
}

void Parser::expectKeyword(const string& keyword) {
    if (!matchKeyword(keyword)) {
        error("Expected keyword '" + keyword + "'");
    }
}

void Parser::expectDelimiter(const string& delim) {
    if (!matchDelimiter(delim)) {
        error("Expected delimiter '" + delim + "'");
    }
}

void Parser::error(const string& message) {
    Token current = getCurrentToken();
    cerr << "Syntax error at line " << current.line
        << ", column " << current.column
        << ": " << message << endl;
    cerr << "  Got: " << current.lexeme << endl;
    hasError = true;
}

shared_ptr<ASTNode> Parser::parseProgram() {
    auto node = make_shared<ASTNode>(NodeType::PROGRAM, "program", getCurrentToken().line);

    expectKeyword("program");
    Token idToken = getCurrentToken();
    if (match(TokenType::IDENTIFIER)) {
        auto idNode = make_shared<ASTNode>(NodeType::VARIABLE, idToken.lexeme, idToken.line);
        node->addChild(idNode);
    }
    else {
        error("Expected program name");
    }
    expectDelimiter(";");

    auto blockNode = parseBlock();
    node->addChild(blockNode);

    expectDelimiter(".");

    return node;
}

shared_ptr<ASTNode> Parser::parseBlock() {
    auto node = make_shared<ASTNode>(NodeType::BLOCK, "block", getCurrentToken().line);

    auto varDeclNode = parseVarDecl();
    if (varDeclNode) {
        node->addChild(varDeclNode);
    }

    expectKeyword("begin");

    auto stmtListNode = parseStatementList();
    node->addChild(stmtListNode);

    expectKeyword("end");

    return node;
}

shared_ptr<ASTNode> Parser::parseVarDecl() {
    if (!matchKeyword("var")) {
        return nullptr;
    }

    auto node = make_shared<ASTNode>(NodeType::VAR_DECL, "var", getCurrentToken().line);

    auto varListNode = parseVarList();
    node->addChild(varListNode);

    expectDelimiter(":");
    expectKeyword("real");
    expectDelimiter(";");

    return node;
}

shared_ptr<ASTNode> Parser::parseVarList() {
    auto node = make_shared<ASTNode>(NodeType::VAR_LIST, "var_list", getCurrentToken().line);

    Token idToken = getCurrentToken();
    if (match(TokenType::IDENTIFIER)) {
        auto idNode = make_shared<ASTNode>(NodeType::VARIABLE, idToken.lexeme, idToken.line);
        node->addChild(idNode);
    }
    else {
        error("Expected identifier in var list");
        return node;
    }

    while (matchDelimiter(",")) {
        idToken = getCurrentToken();
        if (match(TokenType::IDENTIFIER)) {
            auto idNode = make_shared<ASTNode>(NodeType::VARIABLE, idToken.lexeme, idToken.line);
            node->addChild(idNode);
        }
        else {
            error("Expected identifier after comma");
        }
    }

    return node;
}

shared_ptr<ASTNode> Parser::parseStatementList() {
    auto node = make_shared<ASTNode>(NodeType::STATEMENT_LIST, "stmt_list", getCurrentToken().line);

    auto stmtNode = parseStatement();
    if (stmtNode) {
        node->addChild(stmtNode);
    }

    while (matchDelimiter(";")) {
        stmtNode = parseStatement();
        if (stmtNode) {
            node->addChild(stmtNode);
        }
    }

    return node;
}

shared_ptr<ASTNode> Parser::parseStatement() {
    Token current = getCurrentToken();

    if (current.type == TokenType::IDENTIFIER) {
        return parseAssignment();
    }
    else if (matchKeyword("while")) {
        return parseWhileLoop();
    }
    else if (matchKeyword("read")) {
        return parseReadStatement();
    }
    else if (matchKeyword("write")) {
        return parseWriteStatement();
    }

    return nullptr;
}

shared_ptr<ASTNode> Parser::parseAssignment() {
    auto node = make_shared<ASTNode>(NodeType::ASSIGNMENT, ":=", getCurrentToken().line);

    Token idToken = getCurrentToken();
    if (match(TokenType::IDENTIFIER)) {
        auto varNode = make_shared<ASTNode>(NodeType::VARIABLE, idToken.lexeme, idToken.line);
        node->addChild(varNode);
    }
    else {
        error("Expected identifier in assignment");
        return node;
    }

    if (!matchOperator(":=")) {
        error("Expected ':=' in assignment");
        return node;
    }

    auto exprNode = parseExpression();
    node->addChild(exprNode);

    return node;
}

shared_ptr<ASTNode> Parser::parseWhileLoop() {
    auto node = make_shared<ASTNode>(NodeType::WHILE_LOOP, "while", getCurrentToken().line);

    auto exprNode = parseExpression();
    node->addChild(exprNode);

    if (!matchKeyword("do")) {
        error("Expected 'do' after while condition");
        return node;
    }

    if (matchKeyword("begin")) {
        auto stmtListNode = parseStatementList();
        node->addChild(stmtListNode);
        expectKeyword("end");
    }
    else {
        auto stmtNode = parseStatement();
        if (stmtNode) {
            node->addChild(stmtNode);
        }
    }

    return node;
}

shared_ptr<ASTNode> Parser::parseReadStatement() {
    auto node = make_shared<ASTNode>(NodeType::READ_STATEMENT, "read", getCurrentToken().line);

    expectDelimiter("(");

    Token idToken = getCurrentToken();
    if (match(TokenType::IDENTIFIER)) {
        auto varNode = make_shared<ASTNode>(NodeType::VARIABLE, idToken.lexeme, idToken.line);
        node->addChild(varNode);
    }
    else {
        error("Expected identifier in read");
    }

    expectDelimiter(")");

    return node;
}

shared_ptr<ASTNode> Parser::parseWriteStatement() {
    auto node = make_shared<ASTNode>(NodeType::WRITE_STATEMENT, "write", getCurrentToken().line);

    expectDelimiter("(");

    auto exprNode = parseExpression();
    node->addChild(exprNode);

    expectDelimiter(")");

    return node;
}

shared_ptr<ASTNode> Parser::parseExpression() {
    auto node = make_shared<ASTNode>(NodeType::EXPRESSION, "expr", getCurrentToken().line);

    auto leftExpr = parseArithExpression();
    node->addChild(leftExpr);

    if (matchOperator("<") || matchOperator(">") || matchOperator("<=") ||
        matchOperator(">=") || matchOperator("=") || matchOperator("<>")) {

        string op = tokenizer.getOperators()[getCurrentToken().tableIndex];
        auto compNode = make_shared<ASTNode>(NodeType::BINARY_OP, op, getCurrentToken().line);

        auto rightExpr = parseArithExpression();
        compNode->addChild(leftExpr);
        compNode->addChild(rightExpr);

        return compNode;
    }

    return node;
}

shared_ptr<ASTNode> Parser::parseArithExpression() {
    auto node = make_shared<ASTNode>(NodeType::BINARY_OP, "+", getCurrentToken().line);

    auto termNode = parseTerm();
    node->addChild(termNode);

    while (matchOperator("+") || matchOperator("-")) {
        string op = tokenizer.getOperators()[getCurrentToken().tableIndex];
        auto newNode = make_shared<ASTNode>(NodeType::BINARY_OP, op, getCurrentToken().line);
        newNode->addChild(node);
        auto nextTerm = parseTerm();
        newNode->addChild(nextTerm);
        node = newNode;
    }

    return node;
}

shared_ptr<ASTNode> Parser::parseTerm() {
    auto node = make_shared<ASTNode>(NodeType::BINARY_OP, "*", getCurrentToken().line);

    auto factorNode = parseFactor();
    node->addChild(factorNode);

    while (matchOperator("*") || matchOperator("/")) {
        string op = tokenizer.getOperators()[getCurrentToken().tableIndex];
        auto newNode = make_shared<ASTNode>(NodeType::BINARY_OP, op, getCurrentToken().line);
        newNode->addChild(node);
        auto nextFactor = parseFactor();
        newNode->addChild(nextFactor);
        node = newNode;
    }

    return node;
}

shared_ptr<ASTNode> Parser::parseFactor() {
    Token current = getCurrentToken();

    if (match(TokenType::IDENTIFIER)) {
        return make_shared<ASTNode>(NodeType::VARIABLE, current.lexeme, current.line);
    }

    if (match(TokenType::NUMBER)) {
        return make_shared<ASTNode>(NodeType::NUMBER_LITERAL, current.lexeme, current.line);
    }

    if (matchDelimiter("(")) {
        auto exprNode = parseExpression();
        expectDelimiter(")");
        return exprNode;
    }

    error("Expected factor (identifier, number, or '(')");
    return nullptr;
}

shared_ptr<ASTNode> Parser::parse() {
    auto root = parseProgram();

    if (!hasError && getCurrentToken().type != TokenType::END) {
        error("Unexpected tokens after program end");
    }

    return root;
}

void Parser::printAST(shared_ptr<ASTNode> node, int depth) {
    if (!node) return;

    string indent(depth * 2, ' ');

    string typeStr;
    switch (node->type) {
    case NodeType::PROGRAM: typeStr = "PROGRAM"; break;
    case NodeType::BLOCK: typeStr = "BLOCK"; break;
    case NodeType::VAR_DECL: typeStr = "VAR_DECL"; break;
    case NodeType::VAR_LIST: typeStr = "VAR_LIST"; break;
    case NodeType::ASSIGNMENT: typeStr = "ASSIGNMENT"; break;
    case NodeType::WHILE_LOOP: typeStr = "WHILE_LOOP"; break;
    case NodeType::READ_STATEMENT: typeStr = "READ"; break;
    case NodeType::WRITE_STATEMENT: typeStr = "WRITE"; break;
    case NodeType::STATEMENT_LIST: typeStr = "STATEMENT_LIST"; break;
    case NodeType::BINARY_OP: typeStr = "BINARY_OP"; break;
    case NodeType::VARIABLE: typeStr = "VAR"; break;
    case NodeType::NUMBER_LITERAL: typeStr = "NUM"; break;
    case NodeType::EXPRESSION: typeStr = "EXPR"; break;
    default: typeStr = "UNKNOWN";
    }

    cout << indent << "[" << typeStr;
    if (!node->value.empty()) {
        cout << " " << node->value;
    }
    cout << "]" << endl;

    for (const auto& child : node->children) {
        printAST(child, depth + 1);
    }
}

bool Parser::semanticAnalysis(shared_ptr<ASTNode> root) {
    if (!root) return false;

    hasError = false;
    symbolTable.clear();

    function<void(shared_ptr<ASTNode>)> collectDeclarations =
        [&](shared_ptr<ASTNode> node) {
        if (!node) return;

        if (node->type == NodeType::VAR_DECL) {
            for (const auto& child : node->children) {
                if (child->type == NodeType::VAR_LIST) {
                    for (const auto& var : child->children) {
                        if (var->type == NodeType::VARIABLE) {
                            for (const auto& sym : symbolTable) {
                                if (sym.name == var->value) {
                                    cerr << "Semantic error at line " << var->line
                                        << ": duplicate declaration of variable '"
                                        << var->value << "'" << endl;
                                    hasError = true;
                                    return;
                                }
                            }
                            symbolTable.push_back({ var->value, "real", var->line });
                        }
                    }
                }
            }
        }

        for (const auto& child : node->children) {
            collectDeclarations(child);
        }
        };

    function<void(shared_ptr<ASTNode>)> checkUsage =
        [&](shared_ptr<ASTNode> node) {
        if (!node) return;

        if (node->type == NodeType::VARIABLE) {
            bool found = false;
            for (const auto& sym : symbolTable) {
                if (sym.name == node->value) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                cerr << "Semantic error at line " << node->line
                    << ": undeclared identifier '" << node->value << "'" << endl;
                hasError = true;
            }
        }

        for (const auto& child : node->children) {
            checkUsage(child);
        }
        };

    collectDeclarations(root);
    if (!hasError) {
        checkUsage(root);
    }

    return !hasError;
}

void Parser::printSymbolTable() const {
    cout << "\n=== SYMBOL TABLE ===" << endl;
    cout << left << setw(20) << "Name"
        << setw(10) << "Type"
        << setw(10) << "Line" << endl;
    cout << string(40, '-') << endl;
    for (const auto& sym : symbolTable) {
        cout << left << setw(20) << sym.name
            << setw(10) << sym.type
            << setw(10) << sym.line << endl;
    }
}