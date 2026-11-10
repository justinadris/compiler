#pragma once

#include <string>
#include <unordered_map>

#include "builder.h"

extern std::unordered_map<std::string, double> symbolTable;

using namespace std;

enum class PrimitiveType
{
    BYTE, WORD, DWORD, QWORD
};

struct TrackSymbol
{
    vector<vector<string>>  scope_stack;

    void enter_scope();
    void exit_scope();

    vector<string> variables;
    bool exists(string name);
    void push(string name);
};


class Node
{
public:
    virtual ~Node() = default;
    virtual string build(Builder& builder, TrackSymbol& tracker) = 0;
};

class Constant  : public Node
{
public:
    string val;

    Constant(string value);
    
    string build(Builder& bulder, TrackSymbol& tracker) override;
};

class BinaryOp : public Node
{
public:
    char op;
    Node* left;
    Node* right;
    
    BinaryOp(char op, Node* left, Node* right);
    ~BinaryOp();

    string build(Builder& bulder, TrackSymbol& tracker) override;
};

class DeclareVariable : public Node
{
public:
    string name;
    Node* expr;
    DeclareVariable(string name, Node* expr = nullptr);
    ~DeclareVariable();
    string build(Builder& bulder, TrackSymbol& tracker) override;
};

class GetVariable : public Node
{
public:
    string name;
    
    GetVariable(string name);

    string build(Builder& bulder, TrackSymbol& tracker) override;
};

class Assignment : public Node
{
public:
    GetVariable* left;
    Node* right;

    Assignment(GetVariable* left, Node* right);
    ~Assignment();

    string build(Builder& bulder, TrackSymbol& tracker) override;
};

class Function : public Node
{
public:
    string name;
    vector<Node*> block;
    Function(string name, vector<Node*>& block);

    string build(Builder& builder, TrackSymbol& tracker) override;

    ~Function();
};

class Return : public Node
{
public:
    Node* expr;
    Return(Node* expr);

    string build(Builder& builder, TrackSymbol& tracker) override;

    ~Return();
};