#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
using namespace std;

enum class TokenType {
    KEYWORD,     // ключевое слово
    IDENTIFIER,  // идентификатор
    NUMBER,      // число (целое)
    OPERATOR,    // оператор
    DELIMITER,   // разделитель
    UNKNOWN,     // неизвестный
    END          // конец файла
};

struct Token {
    TokenType type;
    int tableIndex;
    string lexeme;
    int line;
    int column;

    Token() : type(TokenType::END), tableIndex(-1), line(0), column(0) {}
    Token(TokenType t, int idx, const string& lex, int l, int c)
        : type(t), tableIndex(idx), lexeme(lex), line(l), column(c) {}
};

class Tokenizer {
private:
    ifstream source;
    string currentLine;
    int lineNumber;
    int columnNumber;
    int posInLine;
    char currentChar;

    vector<string> keywords;
    vector<string> identifiers;
    vector<string> numbers;
    vector<string> operators;
    vector<string> delimiters;

    map<string, int> keywordMap;
    map<string, int> operatorMap;
    map<string, int> delimiterMap;

    void getNextChar();
    void skipWhitespace();
    bool isLetter(char c) const;
    bool isDigit(char c) const;
    bool isOperatorChar(char c) const;
    bool isDelimiter(char c) const;

    Token readIdentifier();
    Token readNumber();
    Token readOperator();
    Token readDelimiter();

public:
    Tokenizer();
    ~Tokenizer();

    bool openFile(const string& filename);
    void closeFile();
    Token getNextToken();
    void printTables() const;
    void printTokens(const vector<Token>& tokens) const;

    const vector<string>& getKeywords() const { return keywords; }
    const vector<string>& getIdentifiers() const { return identifiers; }
    const vector<string>& getNumbers() const { return numbers; }
    const vector<string>& getOperators() const { return operators; }
    const vector<string>& getDelimiters() const { return delimiters; }
};

#endif