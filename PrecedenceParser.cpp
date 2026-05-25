#include "PrecedenceParser.h"
#include <iostream>
#include <iomanip>
#include <cctype>
using namespace std;

PrecedenceParser::PrecedenceParser(Tokenizer& tokenizer)
    : currentPos(0), hasError(false) {

    Token token;
    do {
        token = tokenizer.getNextToken();
        tokens.push_back(token);
    } while (token.type != TokenType::END);

    buildPrecedenceTable();
}

PrecedenceParser::~PrecedenceParser() {}

void PrecedenceParser::buildPrecedenceTable() {
    
    precedenceTable[{"program", "id"}] = '<';
    precedenceTable[{"id", ";"}] = '>';
    precedenceTable[{";", "var"}] = '<';
    precedenceTable[{";", "begin"}] = '<';

    precedenceTable[{"var", "id"}] = '<';
    precedenceTable[{"id", ","}] = '>';
    precedenceTable[{",", "id"}] = '<';
    precedenceTable[{"id", ":"}] = '>';
    precedenceTable[{":", "integer"}] = '<';
    precedenceTable[{"integer", ";"}] = '>';

    precedenceTable[{"begin", "id"}] = '<';
    precedenceTable[{"begin", "while"}] = '<';
    precedenceTable[{"begin", "read"}] = '<';
    precedenceTable[{"begin", "write"}] = '<';
    precedenceTable[{"begin", "end"}] = '>';
    precedenceTable[{"id", ";"}] = '>';
    precedenceTable[{"number", ";"}] = '>';
    precedenceTable[{")", ";"}] = '>';
    precedenceTable[{";", "id"}] = '<';
    precedenceTable[{";", "while"}] = '<';
    precedenceTable[{";", "read"}] = '<';
    precedenceTable[{";", "write"}] = '<';
    precedenceTable[{";", "end"}] = '>';
    precedenceTable[{"end", "."}] = '>';

    precedenceTable[{"id", ":="}] = '<';
    precedenceTable[{":=", "id"}] = '<';
    precedenceTable[{":=", "number"}] = '<';
    precedenceTable[{":=", "("}] = '<';

    precedenceTable[{"while", "id"}] = '<';
    precedenceTable[{"while", "number"}] = '<';
    precedenceTable[{"while", "("}] = '<';
    precedenceTable[{"id", "do"}] = '>';
    precedenceTable[{"number", "do"}] = '>';
    precedenceTable[{")", "do"}] = '>';
    precedenceTable[{"do", "id"}] = '<';
    precedenceTable[{"do", "while"}] = '<';
    precedenceTable[{"do", "read"}] = '<';
    precedenceTable[{"do", "write"}] = '<';
    precedenceTable[{"do", "begin"}] = '<';

    precedenceTable[{"read", "("}] = '<';
    precedenceTable[{"write", "("}] = '<';
    precedenceTable[{"(", "id"}] = '<';
    precedenceTable[{"(", "number"}] = '<';
    precedenceTable[{"id", ")"}] = '>';
    precedenceTable[{"number", ")"}] = '>';
    precedenceTable[{")", ";"}] = '>';
    precedenceTable[{")", "end"}] = '>';

    precedenceTable[{"id", "+"}] = '>';
    precedenceTable[{"id", "-"}] = '>';
    precedenceTable[{"id", "*"}] = '>';
    precedenceTable[{"id", "/"}] = '>';
    precedenceTable[{"number", "+"}] = '>';
    precedenceTable[{"number", "-"}] = '>';
    precedenceTable[{"number", "*"}] = '>';
    precedenceTable[{"number", "/"}] = '>';
    precedenceTable[{")", "+"}] = '>';
    precedenceTable[{")", "-"}] = '>';
    precedenceTable[{")", "*"}] = '>';
    precedenceTable[{")", "/"}] = '>';
    precedenceTable[{"+", "id"}] = '<';
    precedenceTable[{"+", "number"}] = '<';
    precedenceTable[{"+", "("}] = '<';
    precedenceTable[{"-", "id"}] = '<';
    precedenceTable[{"-", "number"}] = '<';
    precedenceTable[{"-", "("}] = '<';
    precedenceTable[{"*", "id"}] = '<';
    precedenceTable[{"*", "number"}] = '<';
    precedenceTable[{"*", "("}] = '<';
    precedenceTable[{"/", "id"}] = '<';
    precedenceTable[{"/", "number"}] = '<';
    precedenceTable[{"/", "("}] = '<';

    precedenceTable[{"(", "id"}] = '<';
    precedenceTable[{"(", "number"}] = '<';
    precedenceTable[{"(", "("}] = '<';
    precedenceTable[{"id", ")"}] = '>';
    precedenceTable[{"number", ")"}] = '>';
    precedenceTable[{")", ")"}] = '>';
    precedenceTable[{"(", ")"}] = '=';

    precedenceTable[{".", "$"}] = '>';
}

char PrecedenceParser::getPrecedence(const string& left, const string& right) {
    auto it = precedenceTable.find({ left, right });
    if (it != precedenceTable.end()) {
        return it->second;
    }

    return ' ';
}

void PrecedenceParser::error(const string& message, const Token& token) {
    string errMsg = "Syntax error at line " + to_string(token.line) +
        ", column " + to_string(token.column) + ": " + message +
        "\n    Got: " + token.lexeme;
    errors.push_back(errMsg);
    cerr << errMsg << endl;
    hasError = true;
}

shared_ptr<ASTNode> PrecedenceParser::parseProgram() {
    auto programNode = make_shared<ASTNode>(NodeType::PROGRAM, "program");

    if (currentPos >= (int)tokens.size()) {
        error("Unexpected end of file, expected 'program'", Token());
        return programNode;
    }

    if (tokens[currentPos].lexeme != "program") {
        error("Expected 'program', got '" + tokens[currentPos].lexeme + "'", tokens[currentPos]);
        return programNode;
    }
    currentPos++;

    if (currentPos >= (int)tokens.size()) {
        error("Unexpected end of file, expected program name", Token());
        return programNode;
    }

    if (tokens[currentPos].type != TokenType::IDENTIFIER) {
        error("Expected program name, got '" + tokens[currentPos].lexeme + "'", tokens[currentPos]);
        return programNode;
    }
    programNode->value = tokens[currentPos].lexeme;
    currentPos++;

    if (currentPos >= (int)tokens.size()) {
        error("Unexpected end of file, expected ';'", Token());
        return programNode;
    }

    if (tokens[currentPos].lexeme != ";") {
        error("Expected ';' after program name, got '" + tokens[currentPos].lexeme + "'", tokens[currentPos]);
        return programNode;
    }
    currentPos++;

    auto blockNode = parseBlock();
    if (blockNode) {
        programNode->addChild(blockNode);
    }

    if (currentPos < (int)tokens.size()) {
        if (tokens[currentPos].lexeme == ".") {
            currentPos++;
        }
        else {
            error("Expected '.' at end of program, got '" + tokens[currentPos].lexeme + "'", tokens[currentPos]);
        }
    }
    else {
        error("Expected '.' at end of program", Token());
    }

    return programNode;
}

shared_ptr<ASTNode> PrecedenceParser::parseBlock() {
    auto blockNode = make_shared<ASTNode>(NodeType::BLOCK, "block");

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == "var") {
        auto varDeclNode = parseVarDecl();
        if (varDeclNode) {
            blockNode->addChild(varDeclNode);
        }
    }

    if (currentPos >= (int)tokens.size()) {
        error("Unexpected end of file, expected 'begin'", Token());
        return blockNode;
    }

    if (tokens[currentPos].lexeme != "begin") {
        error("Expected 'begin', got '" + tokens[currentPos].lexeme + "'", tokens[currentPos]);
        return blockNode;
    }
    currentPos++;

    auto stmtListNode = parseStatementList();
    if (stmtListNode) {
        blockNode->addChild(stmtListNode);
    }

    if (currentPos >= (int)tokens.size()) {
        error("Unexpected end of file, expected 'end'", Token());
        return blockNode;
    }

    if (tokens[currentPos].lexeme != "end") {
        error("Expected 'end', got '" + tokens[currentPos].lexeme + "'", tokens[currentPos]);
        return blockNode;
    }
    currentPos++;

    return blockNode;
}

shared_ptr<ASTNode> PrecedenceParser::parseVarDecl() {
    auto varDeclNode = make_shared<ASTNode>(NodeType::VAR_DECL, "var");

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == "var") {
        currentPos++;
    }
    else {
        error("Expected 'var'", tokens[currentPos]);
        return varDeclNode;
    }

    auto varListNode = parseVarList();
    if (varListNode) {
        varDeclNode->addChild(varListNode);
    }

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == ":") {
        currentPos++;
    }
    else {
        error("Expected ':' after variable list", tokens[currentPos]);
        return varDeclNode;
    }

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == "integer") {
        currentPos++;
    }
    else {
        error("Expected 'integer' type", tokens[currentPos]);
        return varDeclNode;
    }

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == ";") {
        currentPos++;
    }
    else {
        error("Expected ';' after variable declaration", tokens[currentPos]);
    }

    return varDeclNode;
}

shared_ptr<ASTNode> PrecedenceParser::parseVarList() {
    auto varListNode = make_shared<ASTNode>(NodeType::VAR_LIST, "var_list");

    if (currentPos < (int)tokens.size() && tokens[currentPos].type == TokenType::IDENTIFIER) {
        auto varNode = make_shared<ASTNode>(NodeType::VARIABLE, tokens[currentPos].lexeme, tokens[currentPos].line);
        varListNode->addChild(varNode);
        currentPos++;
    }
    else {
        error("Expected identifier in variable list", tokens[currentPos]);
        return varListNode;
    }

    while (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == ",") {
        currentPos++; 

        if (currentPos < (int)tokens.size() && tokens[currentPos].type == TokenType::IDENTIFIER) {
            auto varNode = make_shared<ASTNode>(NodeType::VARIABLE, tokens[currentPos].lexeme, tokens[currentPos].line);
            varListNode->addChild(varNode);
            currentPos++;
        }
        else {
            error("Expected identifier after comma", tokens[currentPos]);
        }
    }

    return varListNode;
}

shared_ptr<ASTNode> PrecedenceParser::parseStatementList() {
    auto stmtListNode = make_shared<ASTNode>(NodeType::STATEMENT_LIST, "stmt_list");

    while (currentPos < (int)tokens.size()) {
        if (tokens[currentPos].lexeme == "end" || tokens[currentPos].lexeme == ".") {
            break;
        }

        auto stmtNode = parseStatement();
        if (stmtNode) {
            stmtListNode->addChild(stmtNode);
        }

        if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == ";") {
            currentPos++;
        }
        else if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme != "end") {
            break;
        }
    }

    return stmtListNode;
}

shared_ptr<ASTNode> PrecedenceParser::parseStatement() {
    if (currentPos >= (int)tokens.size()) {
        return nullptr;
    }

    Token current = tokens[currentPos];

    if (current.type == TokenType::IDENTIFIER) {
        return parseAssignment();
    }
    else if (current.lexeme == "while") {
        return parseWhileLoop();
    }
    else if (current.lexeme == "read") {
        return parseReadStatement();
    }
    else if (current.lexeme == "write") {
        return parseWriteStatement();
    }

    return nullptr;
}

shared_ptr<ASTNode> PrecedenceParser::parseAssignment() {
    auto assignNode = make_shared<ASTNode>(NodeType::ASSIGNMENT, ":=", tokens[currentPos].line);

    if (currentPos < (int)tokens.size() && tokens[currentPos].type == TokenType::IDENTIFIER) {
        auto varNode = make_shared<ASTNode>(NodeType::VARIABLE, tokens[currentPos].lexeme, tokens[currentPos].line);
        assignNode->addChild(varNode);
        currentPos++;
    }
    else {
        error("Expected identifier in assignment", tokens[currentPos]);
        return assignNode;
    }

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == ":=") {
        currentPos++;
    }
    else {
        error("Expected ':=' in assignment", tokens[currentPos]);
        return assignNode;
    }

    auto exprNode = parseExpression();
    if (exprNode) {
        assignNode->addChild(exprNode);
    }

    return assignNode;
}

shared_ptr<ASTNode> PrecedenceParser::parseWhileLoop() {
    auto whileNode = make_shared<ASTNode>(NodeType::WHILE_LOOP, "while", tokens[currentPos].line);

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == "while") {
        currentPos++;
    }
    else {
        error("Expected 'while'", tokens[currentPos]);
        return whileNode;
    }

    auto condNode = parseExpression();
    if (condNode) {
        whileNode->addChild(condNode);
    }

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == "do") {
        currentPos++;
    }
    else {
        error("Expected 'do' after while condition", tokens[currentPos]);
        return whileNode;
    }

    auto bodyNode = parseStatement();
    if (bodyNode) {
        whileNode->addChild(bodyNode);
    }

    return whileNode;
}

shared_ptr<ASTNode> PrecedenceParser::parseReadStatement() {
    auto readNode = make_shared<ASTNode>(NodeType::READ_STATEMENT, "read", tokens[currentPos].line);

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == "read") {
        currentPos++;
    }
    else {
        error("Expected 'read'", tokens[currentPos]);
        return readNode;
    }

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == "(") {
        currentPos++;
    }
    else {
        error("Expected '(' after read", tokens[currentPos]);
        return readNode;
    }

    if (currentPos < (int)tokens.size() && tokens[currentPos].type == TokenType::IDENTIFIER) {
        auto varNode = make_shared<ASTNode>(NodeType::VARIABLE, tokens[currentPos].lexeme, tokens[currentPos].line);
        readNode->addChild(varNode);
        currentPos++;
    }
    else {
        error("Expected identifier in read statement", tokens[currentPos]);
        return readNode;
    }

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == ")") {
        currentPos++;
    }
    else {
        error("Expected ')' after read", tokens[currentPos]);
    }

    return readNode;
}

shared_ptr<ASTNode> PrecedenceParser::parseWriteStatement() {
    auto writeNode = make_shared<ASTNode>(NodeType::WRITE_STATEMENT, "write", tokens[currentPos].line);

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == "write") {
        currentPos++;
    }
    else {
        error("Expected 'write'", tokens[currentPos]);
        return writeNode;
    }

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == "(") {
        currentPos++;
    }
    else {
        error("Expected '(' after write", tokens[currentPos]);
        return writeNode;
    }

    auto exprNode = parseExpression();
    if (exprNode) {
        writeNode->addChild(exprNode);
    }

    if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == ")") {
        currentPos++;
    }
    else {
        error("Expected ')' after write expression", tokens[currentPos]);
    }

    return writeNode;
}

shared_ptr<ASTNode> PrecedenceParser::parseExpression() {
    auto left = parseArithExpression();

    if (currentPos < (int)tokens.size()) {
        string op = tokens[currentPos].lexeme;
        if (op == "=" || op == "<>" || op == "<" || op == ">" || op == "<=" || op == ">=") {
            currentPos++;
            auto right = parseArithExpression();
            auto binOpNode = make_shared<ASTNode>(NodeType::BINARY_OP, op);
            binOpNode->addChild(left);
            binOpNode->addChild(right);
            return binOpNode;
        }
    }

    auto exprNode = make_shared<ASTNode>(NodeType::EXPRESSION, "expr");
    exprNode->addChild(left);
    return exprNode;
}

shared_ptr<ASTNode> PrecedenceParser::parseArithExpression() {
    auto node = parseTerm();

    while (currentPos < (int)tokens.size()) {
        string op = tokens[currentPos].lexeme;
        if (op == "+" || op == "-") {
            currentPos++;
            auto right = parseTerm();
            auto binOpNode = make_shared<ASTNode>(NodeType::BINARY_OP, op);
            binOpNode->addChild(node);
            binOpNode->addChild(right);
            node = binOpNode;
        }
        else {
            break;
        }
    }

    return node;
}

shared_ptr<ASTNode> PrecedenceParser::parseTerm() {
    auto node = parseFactor();

    while (currentPos < (int)tokens.size()) {
        string op = tokens[currentPos].lexeme;
        if (op == "*" || op == "/") {
            currentPos++;
            auto right = parseFactor();
            auto binOpNode = make_shared<ASTNode>(NodeType::BINARY_OP, op);
            binOpNode->addChild(node);
            binOpNode->addChild(right);
            node = binOpNode;
        }
        else {
            break;
        }
    }

    return node;
}

shared_ptr<ASTNode> PrecedenceParser::parseFactor() {
    if (currentPos >= (int)tokens.size()) {
        error("Unexpected end of input", Token());
        return nullptr;
    }

    Token current = tokens[currentPos];

    if (current.type == TokenType::IDENTIFIER) {
        currentPos++;
        return make_shared<ASTNode>(NodeType::VARIABLE, current.lexeme, current.line);
    }

    if (current.type == TokenType::NUMBER) {
        currentPos++;
        return make_shared<ASTNode>(NodeType::NUMBER_LITERAL, current.lexeme, current.line);
    }

    if (current.lexeme == "(") {
        currentPos++;
        auto exprNode = parseExpression();
        if (currentPos < (int)tokens.size() && tokens[currentPos].lexeme == ")") {
            currentPos++;
        }
        else {
            error("Expected ')'", tokens[currentPos]);
        }
        return exprNode;
    }

    error("Expected factor (identifier, number, or '(')", current);
    return nullptr;
}

bool PrecedenceParser::parse() {
    cout << "\n=== PRECEDENCE PARSER TRACE ===" << endl;
    cout << "Total tokens: " << tokens.size() << endl;

    root = parseProgram();

    if (!hasError && currentPos < (int)tokens.size() && tokens[currentPos].type != TokenType::END) {
        error("Unexpected tokens after program end", tokens[currentPos]);
    }

    if (!hasError) {
        cout << "Parsing completed successfully!" << endl;
        buildSymbolTable();
    }

    return !hasError;
}

void PrecedenceParser::printErrors() const {
    if (!errors.empty()) {
        cout << "\n=== ERRORS ===" << endl;
        for (const auto& err : errors) {
            cout << err << endl;
        }
    }
}

void PrecedenceParser::buildSymbolTable() {
    symbolTable.clear();

    function<void(shared_ptr<ASTNode>)> collectVars = [&](shared_ptr<ASTNode> node) {
        if (!node) return;

        if (node->type == NodeType::VAR_DECL) {
            for (const auto& child : node->children) {
                if (child->type == NodeType::VAR_LIST) {
                    for (const auto& var : child->children) {
                        if (var->type == NodeType::VARIABLE) {
                            bool found = false;
                            for (const auto& sym : symbolTable) {
                                if (sym.name == var->value) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                symbolTable.push_back({ var->value, "integer", var->line });
                            }
                        }
                    }
                }
            }
        }

        for (const auto& child : node->children) {
            collectVars(child);
        }
        };

    collectVars(root);
}

void PrecedenceParser::printSymbolTable() const {
    cout << "\n=== SYMBOL TABLE ===" << endl;
    if (symbolTable.empty()) {
        cout << "(no variables declared)" << endl;
    }
    else {
        cout << left << setw(20) << "Name" << setw(10) << "Type" << setw(10) << "Line" << endl;
        cout << string(40, '-') << endl;
        for (const auto& sym : symbolTable) {
            cout << left << setw(20) << sym.name << setw(10) << sym.type << setw(10) << sym.line << endl;
        }
    }
}

void PrecedenceParser::printAST(shared_ptr<ASTNode> node, int depth) const {
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
        cout << " \"" << node->value << "\"";
    }
    if (node->type == NodeType::VARIABLE || node->type == NodeType::NUMBER_LITERAL) {
        if (node->value.empty()) {
            cout << " \"" << node->value << "\"";
        }
    }
    cout << "]" << endl;

    for (const auto& child : node->children) {
        printAST(child, depth + 1);
    }
}

void PrecedenceParser::printASTree() const {
    if (root) {
        cout << "\n=== ABSTRACT SYNTAX TREE ===" << endl;
        printAST(root, 0);
    }
}