#include "Tokenizer.h"
#include <cctype>
#include <algorithm>
using namespace std;

Tokenizer::Tokenizer()
    : lineNumber(1), columnNumber(0), posInLine(-1), currentChar(' ') {

    keywords = { "program", "var", "real", "begin", "while", "do", "end", "read", "write" };
    for (size_t i = 0; i < keywords.size(); ++i) {
        keywordMap[keywords[i]] = i;
    }

    operators = { ":=", "+", "-", "*", "/", "=", "<", ">", "<=", ">=", "<>" };
    for (size_t i = 0; i < operators.size(); ++i) {
        operatorMap[operators[i]] = i;
    }

    delimiters = { "(", ")", ";", ",", ":", ".", " " };
    for (size_t i = 0; i < delimiters.size(); ++i) {
        delimiterMap[delimiters[i]] = i;
    }
}

Tokenizer::~Tokenizer() {
    closeFile();
}

bool Tokenizer::openFile(const string& filename) {
    source.open(filename);
    if (!source.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return false;
    }
    getNextChar();
    return true;
}

void Tokenizer::closeFile() {
    if (source.is_open()) {
        source.close();
    }
}

void Tokenizer::getNextChar() {
    if (posInLine + 1 >= static_cast<int>(currentLine.length())) {
        if (getline(source, currentLine)) {
            lineNumber++;
            columnNumber = 0;
            posInLine = -1;
            currentLine += '\n';
        }
        else {
            currentChar = EOF;
            return;
        }
    }
    posInLine++;
    columnNumber++;
    currentChar = currentLine[posInLine];
}

void Tokenizer::skipWhitespace() {
    while (currentChar == ' ' || currentChar == '\t' || currentChar == '\n' || currentChar == '\r') {
        getNextChar();
    }
}

bool Tokenizer::isLetter(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Tokenizer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Tokenizer::isOperatorChar(char c) const {
    // Убираем ':' из операторов, он будет обрабатываться как разделитель
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>';
}

bool Tokenizer::isDelimiter(char c) const {
    return c == '(' || c == ')' || c == ';' || c == ',' || c == ':' || c == '.' || c == ' ';
}

Token Tokenizer::readIdentifier() {
    string lexeme;
    int startLine = lineNumber;
    int startColumn = columnNumber;

    while (isLetter(currentChar) || isDigit(currentChar)) {
        lexeme += currentChar;
        getNextChar();
    }

    auto it = keywordMap.find(lexeme);
    if (it != keywordMap.end()) {
        return Token(TokenType::KEYWORD, it->second, 0, lexeme, startLine, startColumn);
    }

    size_t idx = identifiers.size();
    auto idIt = find(identifiers.begin(), identifiers.end(), lexeme);
    if (idIt != identifiers.end()) {
        idx = distance(identifiers.begin(), idIt);
    }
    else {
        identifiers.push_back(lexeme);
    }

    return Token(TokenType::IDENTIFIER, static_cast<int>(idx), 0, lexeme, startLine, startColumn);
}

Token Tokenizer::readNumber() {
    string lexeme;
    int startLine = lineNumber;
    int startColumn = columnNumber;

    while (isDigit(currentChar)) {
        lexeme += currentChar;
        getNextChar();
    }

    if (currentChar == '.') {
        lexeme += currentChar;
        getNextChar();

        if (!isDigit(currentChar)) {
            cerr << "Lexical error: Invalid number format at line " << startLine
                << ", column " << startColumn << endl;
            return Token(TokenType::UNKNOWN, -1, 0, lexeme, startLine, startColumn);
        }

        while (isDigit(currentChar)) {
            lexeme += currentChar;
            getNextChar();
        }
    }

    size_t idx = numbers.size();
    numbers.push_back(lexeme);

    return Token(TokenType::NUMBER, static_cast<int>(idx), 0, lexeme, startLine, startColumn);
}

Token Tokenizer::readOperator() {
    string lexeme;
    int startLine = lineNumber;
    int startColumn = columnNumber;

    lexeme += currentChar;
    getNextChar();

    // Двухсимвольные операторы (:=, <=, >=, <>)
    if ((lexeme[0] == ':' && currentChar == '=') ||
        (lexeme[0] == '<' && currentChar == '=') ||
        (lexeme[0] == '<' && currentChar == '>') ||
        (lexeme[0] == '>' && currentChar == '=')) {
        lexeme += currentChar;
        getNextChar();
        auto it = operatorMap.find(lexeme);
        if (it != operatorMap.end()) {
            return Token(TokenType::OPERATOR, it->second, 0, lexeme, startLine, startColumn);
        }
    }
    // Если это одиночный ':' — возвращаем как разделитель
    else if (lexeme[0] == ':') {
        auto it = delimiterMap.find(lexeme);
        if (it != delimiterMap.end()) {
            return Token(TokenType::DELIMITER, it->second, 0, lexeme, startLine, startColumn);
        }
    }
    // Односимвольные операторы из списка (+, -, *, /, =, <, >)
    else {
        auto it = operatorMap.find(lexeme);
        if (it != operatorMap.end()) {
            return Token(TokenType::OPERATOR, it->second, 0, lexeme, startLine, startColumn);
        }
    }

    cerr << "Lexical error: Unknown operator at line " << startLine
        << ", column " << startColumn << endl;
    return Token(TokenType::UNKNOWN, -1, 0, lexeme, startLine, startColumn);
}

Token Tokenizer::readDelimiter() {
    string lexeme(1, currentChar);
    int startLine = lineNumber;
    int startColumn = columnNumber;

    getNextChar();

    auto it = delimiterMap.find(lexeme);
    if (it != delimiterMap.end()) {
        return Token(TokenType::DELIMITER, it->second, 0, lexeme, startLine, startColumn);
    }

    return Token(TokenType::UNKNOWN, -1, 0, lexeme, startLine, startColumn);
}

Token Tokenizer::getNextToken() {
    skipWhitespace();

    if (currentChar == EOF) {
        return Token(TokenType::END, -1, 0, "EOF", lineNumber, columnNumber);
    }

    if (isLetter(currentChar)) {
        return readIdentifier();
    }

    if (isDigit(currentChar)) {
        return readNumber();
    }

    if (isOperatorChar(currentChar)) {
        return readOperator();
    }

    if (isDelimiter(currentChar)) {
        return readDelimiter();
    }

    string lexeme(1, currentChar);
    int startLine = lineNumber;
    int startColumn = columnNumber;
    cerr << "Lexical error: Unknown character '" << currentChar
        << "' at line " << startLine << ", column " << startColumn << endl;
    getNextChar();
    return Token(TokenType::UNKNOWN, -1, 0, lexeme, startLine, startColumn);
}

void Tokenizer::printTables() const {
    cout << "\n=== TABLES OF LEXEMES ===" << endl;

    cout << "\nTable 1 - Keywords:" << endl;
    for (size_t i = 0; i < keywords.size(); ++i) {
        cout << "  [" << i << "] = \"" << keywords[i] << "\"" << endl;
    }

    cout << "\nTable 2 - Identifiers:" << endl;
    for (size_t i = 0; i < identifiers.size(); ++i) {
        cout << "  [" << i << "] = \"" << identifiers[i] << "\"" << endl;
    }

    cout << "\nTable 3 - Numbers:" << endl;
    for (size_t i = 0; i < numbers.size(); ++i) {
        cout << "  [" << i << "] = \"" << numbers[i] << "\"" << endl;
    }

    cout << "\nTable 4 - Operators:" << endl;
    for (size_t i = 0; i < operators.size(); ++i) {
        cout << "  [" << i << "] = \"" << operators[i] << "\"" << endl;
    }

    cout << "\nTable 5 - Delimiters:" << endl;
    for (size_t i = 0; i < delimiters.size(); ++i) {
        cout << "  [" << i << "] = \"" << delimiters[i] << "\"" << endl;
    }
}

void Tokenizer::printTokens(const vector<Token>& tokens) const {
    cout << "\n=== TOKEN SEQUENCE ===" << endl;

    for (const auto& token : tokens) {
        if (token.type == TokenType::END) break;

        string typeStr;
        string value;

        switch (token.type) {
        case TokenType::KEYWORD:
            typeStr = "KEYWORD";
            if (token.tableIndex >= 0 && token.tableIndex < (int)keywords.size())
                value = keywords[token.tableIndex];
            break;
        case TokenType::IDENTIFIER:
            typeStr = "IDENTIFIER";
            if (token.tableIndex >= 0 && token.tableIndex < (int)identifiers.size())
                value = identifiers[token.tableIndex];
            break;
        case TokenType::NUMBER:
            typeStr = "NUMBER";
            if (token.tableIndex >= 0 && token.tableIndex < (int)numbers.size())
                value = numbers[token.tableIndex];
            break;
        case TokenType::OPERATOR:
            typeStr = "OPERATOR";
            if (token.tableIndex >= 0 && token.tableIndex < (int)operators.size())
                value = operators[token.tableIndex];
            break;
        case TokenType::DELIMITER:
            typeStr = "DELIMITER";
            if (token.tableIndex >= 0 && token.tableIndex < (int)delimiters.size())
                value = delimiters[token.tableIndex];
            break;
        default:
            continue;
        }

        cout << typeStr << ": \"" << value << "\" [line:" << token.line << ", col:" << token.column << "]" << endl;
    }
}