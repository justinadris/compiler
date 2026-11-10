/*
    overview: 
        Read file contents to memory

        Lexical analysis
            tokens
            lexer

        Syntax analysis  
            parser        

        Semantic analysis
            type checking
            scope resolution
            function validation
            symbol table
        
        Intermediate representation
        
        Optimization
*/

/*
    No need to compile to object files, source code -> executable
    
    goals for now:
        custom functions with parameters
        FuncDefNode class
        FuncCallNode class
        TrackSymbol needs to hold a vector (stack) of maps
            everytime a function is called, push a fresh, empty map (stack frame)   
            any new variables and parameters are saved only to the top map
            when a function finishes, pop the map off the stack
        

*/

/*
    types of AST nodes:
        Leaf nodes
            Constant nodes
            Identifier nodes

        Expression nodes
            Binary op (x + y)
            Unary op (!x)
            Assignment nodes (x = 10)
    
        Statement nodes
            If-statement nodes (condition, then_block (block node), else_block (block node))
            Loop nodes (condition, body (block node))
            Block nodes (scopes)

        Declaration nodes
            Function definition nodes (type, name, params, body (block node))
            Variable declaration nodes (type, name)
*/

/*
    IR code representation
    add {
        param a
        param b
        t0 = a + b
        ret t0
    }

    main {
        t0 = 4
        t1 = 7
        param t0
        param t1
        t2 = call add, 2
        x = t2
    }

*/


#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include <cctype>
#include <vector>
#include <algorithm>

#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "builder.h"

using namespace std;

string ReadFile(string filename)
{
    ifstream file(filename);
    stringstream buffer;
    
    buffer << file.rdbuf();
    string code = buffer.str();

    file.close();
    return code;
}

string TokenTypeToString(TokenType type)
{
    switch (type)
    {
        case TokenType::PLUS:        return "(PLUS)";
        case TokenType::MINUS:       return "(MINUS)";
        case TokenType::STAR:        return "(STAR)";
        case TokenType::SLASH:       return "(SLASH)";
        case TokenType::EQUAL:       return "(EQUAL)";
        case TokenType::SEMICOLON:   return "(SEMICOLON)";
        case TokenType::LEFT_PAREN:  return "(LEFT_PAREN)";
        case TokenType::RIGHT_PAREN: return "(RIGHT_PAREN)";
        case TokenType::LEFT_BRACE:  return "(LEFT_BRACE)";
        case TokenType::RIGHT_BRACE: return "(RIGHT_BRACE)";
        case TokenType::IDENTIFIER:  return "(IDENTIFIER)";
        case TokenType::NUMBER:      return "(NUMBER)";
        case TokenType::ENTRY_POINT: return "(ENTRY_POINT)";
        case TokenType::BYTE:        return "(BYTE)";
        case TokenType::PRINT:       return "(PRINT)";
        case TokenType::END_OF_FILE: return "";
        default:                     return "UNKNOWN SYMBOL";
    }
}


int main()
{
    string code = ReadFile("code.bin");

    Builder builder;
    TrackSymbol tracker;

    Lexer lexer(code);
    lexer.scan();
    
    Parser parser(lexer.tokens);
    vector<Node*> program = parser.parse(); 

    for (Node* statement : program)
        statement->build(builder, tracker);
    
    for (IRFunction& fun : builder.functions)
    {
        cout << fun.name  << " {\n";

        for (string& instr : fun.instructions)
            cout << "    " << instr << '\n';
        
        cout << "}\n\n";
    }

    for (Node* statement : program)
        delete statement;

}  
