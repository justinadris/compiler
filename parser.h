#pragma once

#include <string>
#include <vector>

#include "lexer.h"
#include "ast.h"

class Parser 
{
public:
    vector<Token> tokens;
    int current = 0;

    Parser(vector<Token> tokens);

    Token peek();
    bool isAtEnd();
    Token advance();
    bool check(TokenType type);
    Token consume(TokenType type, string msg);

    Node* primary();
    Node* term();
    Node* expression();

    Node* ret();
    Node* assign();
    Node* declare();
    
    vector<Node*> parse();
    vector<Node*> parseBlock();
};