#include "Tokenizer.h"
#include <cctype>
#include <algorithm>
using namespace std;

Tokenizer::Tokenizer()
    : lineNumber(1), columnNumber(0), posInLine(-1), currentChar(' ') {

    keywords = { "program", "var", "integer", "begin", "while", "do", "end", "read", "write" };
    for (size_t i = 0; i < keywords.size(); ++i) {
        keywordMap[keywords[i]] = static_cast<int>(i);
    }

    operators = { ":=", "+", "-", "*", "/", "=", "<", ">", "<=", ">=", "<>" };
    for (size_t i = 0; i < operators.size(); ++i) {
        operatorMap[operators[i]] = static_cast<int>(i);
    }

    delimiters = { "(", ")", ";", ",", ":", ".", " " };
    for (size_t i = 0; i < delimiters.size(); ++i) {
        delimiterMap[delimiters[i]] = static_cast<int>(i);
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
    return c == '+' || c == '-' || c == '*' || c == '/' || c == ':' || c == '=' || c == '<' || c == '>';
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
        return Token(TokenType::KEYWORD, it->second, lexeme, startLine, startColumn);
    }

    int idx = static_cast<int>(identifiers.size());
    auto idIt = find(identifiers.begin(), identifiers.end(), lexeme);
    if (idIt != identifiers.end()) {
        idx = static_cast<int>(distance(identifiers.begin(), idIt));
    }
    else {
        identifiers.push_back(lexeme);
    }

    return Token(TokenType::IDENTIFIER, idx, lexeme, startLine, startColumn);
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
        cerr << "Lexical error at line " << startLine << ", column " << startColumn
            << ": Floating point numbers are not supported (integer only)" << endl;
        return Token(TokenType::UNKNOWN, -1, lexeme, startLine, startColumn);
    }

    int idx = static_cast<int>(numbers.size());
    numbers.push_back(lexeme);

    return Token(TokenType::NUMBER, idx, lexeme, startLine, startColumn);
}

Token Tokenizer::readOperator() {
    string lexeme;
    int startLine = lineNumber;
    int startColumn = columnNumber;

    lexeme += currentChar;
    getNextChar();

    if ((lexeme[0] == ':' && currentChar == '=') ||
        (lexeme[0] == '<' && currentChar == '=') ||
        (lexeme[0] == '<' && currentChar == '>') ||
        (lexeme[0] == '>' && currentChar == '=')) {
        lexeme += currentChar;
        getNextChar();
        auto it = operatorMap.find(lexeme);
        if (it != operatorMap.end()) {
            return Token(TokenType::OPERATOR, it->second, lexeme, startLine, startColumn);
        }
    }

    if (lexeme[0] == ':') {
        auto it = delimiterMap.find(lexeme);
        if (it != delimiterMap.end()) {
            return Token(TokenType::DELIMITER, it->second, lexeme, startLine, startColumn);
        }
    }

    auto it = operatorMap.find(lexeme);
    if (it != operatorMap.end()) {
        return Token(TokenType::OPERATOR, it->second, lexeme, startLine, startColumn);
    }

    cerr << "Lexical error at line " << startLine << ", column " << startColumn
        << ": Unknown operator '" << lexeme << "'" << endl;
    return Token(TokenType::UNKNOWN, -1, lexeme, startLine, startColumn);
}

Token Tokenizer::readDelimiter() {
    string lexeme(1, currentChar);
    int startLine = lineNumber;
    int startColumn = columnNumber;

    getNextChar();

    auto it = delimiterMap.find(lexeme);
    if (it != delimiterMap.end()) {
        return Token(TokenType::DELIMITER, it->second, lexeme, startLine, startColumn);
    }

    return Token(TokenType::UNKNOWN, -1, lexeme, startLine, startColumn);
}

Token Tokenizer::getNextToken() {
    skipWhitespace();

    if (currentChar == EOF) {
        return Token(TokenType::END, -1, "EOF", lineNumber, columnNumber);
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
    cerr << "Lexical error at line " << startLine << ", column " << startColumn
        << ": Unknown character '" << currentChar << "'" << endl;
    getNextChar();
    return Token(TokenType::UNKNOWN, -1, lexeme, startLine, startColumn);
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
        switch (token.type) {
        case TokenType::KEYWORD:   typeStr = "KEYWORD";    break;
        case TokenType::IDENTIFIER:typeStr = "IDENTIFIER"; break;
        case TokenType::NUMBER:    typeStr = "NUMBER";     break;
        case TokenType::OPERATOR:  typeStr = "OPERATOR";   break;
        case TokenType::DELIMITER: typeStr = "DELIMITER";  break;
        default: continue;
        }
        cout << typeStr << ": \"" << token.lexeme << "\" [line:" << token.line << ", col:" << token.column << "]" << endl;
    }
}