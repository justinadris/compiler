#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

char* read_file(char* filename) {
    FILE* file = fopen(filename, "rb");

    fseek(file, 0, SEEK_END);
    int len = ftell(file);
    rewind(file);

    char* buffer = malloc(len + 1);
    fread(buffer, 1, len, file);
    buffer[len] = '\0';

    fclose(file);
    return buffer;
}

typedef struct Builder {
    int size;
    int cap;
    char** instructions;
    int counter;
} Builder;

Builder* create_builder() {
    Builder* b = malloc(sizeof(Builder));
    b->cap = 10;
    b->size = 0;
    b->instructions = malloc(sizeof(char*) * b->cap);
    b->counter = 0;
    return b;
}

char* get_new_t(Builder* b) {
    char buffer[8];
    snprintf(buffer, 8, "t%i", b->counter++);
    return _strdup(buffer);
}

void emit_instruction(Builder* b, char* instr) {
    if (b->size >= b->cap) {
        b->cap *= 2;
        char** tmp = realloc(b->instructions, sizeof(char*) * b->cap);
        if (!tmp) {
            printf("failed to realloc\n");
            exit(1);
        }
        b->instructions = tmp;
    }
    b->instructions[b->size++] = instr;
}

void log_instructions(Builder* b) {
    for (int i = 0; i < b->size; i++)
        printf("%s\n", b->instructions[i]);
}

void delete_builder(Builder* b) {
    for (int i = 0; i < b->size; i++)
        free(b->instructions[i]);
    free(b->instructions);
    free(b);
}

typedef enum NodeType {
    CONSTANT, BINARY_OP, 
    DECLARE_VAR, GET_VAR
} NodeType;

typedef struct Node {
    NodeType type;
    
    union {
        struct {
            char* val;
        } constant;
        
        struct {
            char op;
            struct Node* left;
            struct Node* right;
        } binary_op;

        struct {
            char* name;
            struct Node* expr;
        } declare_var;

        struct {
            char* name;
        } get_var;
    } data;
} Node;

Node* create_constant(double val) {
    Node* n = malloc(sizeof(Node));
    n->type = CONSTANT;
    char buffer[16];
    snprintf(buffer, 16, "%.6g", val);
    n->data.constant.val = _strdup(buffer);
    return n;
}

Node* create_binary_op(char op, Node* left, Node* right) {
    Node* n = malloc(sizeof(Node));
    n->type = BINARY_OP;
    n->data.binary_op.op = op;
    n->data.binary_op.left = left;
    n->data.binary_op.right = right;
    return n;
}

Node* create_declare_var(char* name, Node* expr) {
    Node* n = malloc(sizeof(Node));
    n->type = DECLARE_VAR;
    n->data.declare_var.name = _strdup(name);
    n->data.declare_var.expr = expr;
    return n;
}

Node* create_get_var(char* name) {
    Node* n = malloc(sizeof(Node));
    n->type = GET_VAR;
    n->data.get_var.name = _strdup(name);
    return  n;
}

char* build_node(Node* n, Builder* b) {

    if (n->type == CONSTANT) {
        return _strdup(n->data.constant.val);
    }
    else if (n->type == BINARY_OP) {
        char* left_val = build_node(n->data.binary_op.left, b);
        char* right_val = build_node(n->data.binary_op.right, b);  
      
        char* t = get_new_t(b);

        int len = strlen(left_val) + strlen(right_val) + strlen(t) + 10;
    
        char* instr = malloc(len);
        snprintf(instr, len, "%s = %s %c %s", t, left_val, n->data.binary_op.op, right_val);
        emit_instruction(b, instr);
 
        free(left_val);
        free(right_val);
        return t;
    }
    else if (n->type == DECLARE_VAR) {
        char* expr = build_node(n->data.declare_var.expr, b);

        int len = strlen(n->data.declare_var.name) + strlen(expr) + 5;
        char* instr = malloc(len);
        snprintf(instr, len, "%s = %s", n->data.declare_var.name, expr);
        emit_instruction(b, instr);
        
        free(expr);
        return _strdup(n->data.declare_var.name);
    }
    else if (n->type == GET_VAR) {
        return _strdup(n->data.get_var.name);
    }
    printf("unknown node type\n");
    exit(1);
}

void delete_node(Node* n) {
 
    if (n->type == CONSTANT) {
        free(n->data.constant.val);
    }
    else if (n->type == BINARY_OP) {
        delete_node(n->data.binary_op.left);
        delete_node(n->data.binary_op.right);
    }
    else if (n->type == DECLARE_VAR) {
        free(n->data.declare_var.name);
        delete_node(n->data.declare_var.expr);
    }
    else if (n->type == GET_VAR) {
        free(n->data.get_var.name);
    }
    free(n);
}

typedef enum TokenType {
    PLUS, MINUS, STAR, SLASH,
    EQUAL, LEFT_PAREN, RIGHT_PAREN, SEMICOLON,
    IDENTIFIER, NUMBER, INT,
    END_OF_FILE
} TokenType;

typedef struct Token {
    TokenType type;
    char* lexeme;
} Token;

typedef struct Lexer {
    int current;
    char* source;
    Token** tokens;
    int size;
    int cap;
} Lexer;

Lexer* create_lexer(char* source) {
    Lexer* l = malloc(sizeof(Lexer));
    l->cap = 100;
    l->size = 0;
    l->current = 0;
    l->tokens = malloc(sizeof(Token*) * l->cap);
    l->source = source;
    return l;
}

void emit_token(Lexer* l, TokenType type, char* lexeme) {
    if (l->size >= l->cap) {
        l->cap *= 2;
        Token** tmp = realloc(l->tokens, sizeof(Token*) * l->cap);
        if (!tmp) {
            printf("failed to realloc\n");
            exit(1);
        }
        l->tokens = tmp;
    }
    Token* t = malloc(sizeof(Token));
    t->type = type;
    t->lexeme = lexeme;
    l->tokens[l->size++] = t;
}

bool isAtEnd(Lexer* l) {
    return l->current >= strlen(l->source);
}

char peek(Lexer* l) {
    return l->source[l->current];
}

char advance(Lexer* l) {
    if (!isAtEnd(l))
        l->current++;
    return l->source[l->current - 1];    
}

void scan(Lexer* l) {
    while (!isAtEnd(l)) {
        int start = l->current;
        char c = advance(l);
        
        if (isspace(c))
            continue;
        
        if (isalpha(c)) {
            while (isalnum(peek(l)))
                advance(l);
            
            int len = l->current - start;
            char* name = calloc(len + 1, sizeof(char));
            int idx = 0;
            for (int i = start; i < l->current; i++)
                name[idx++] = l->source[i]; 
        
            if (strcmp(name, "int") == 0)
                emit_token(l, INT, name);
            else
                emit_token(l, IDENTIFIER, name);
        }
        else if (isdigit(c)) {
            while (isdigit(peek(l)))
                advance(l);

            int len = l->current - start;
            char* number = calloc(len + 1, sizeof(char));
            int idx = 0;
            for (int i = start; i < l->current; i++)
                number[idx++] = l->source[i];
            
            emit_token(l, NUMBER, number);
        }
        else if (c == '+')
            emit_token(l, PLUS, _strdup("+"));
        else if (c == '-')
            emit_token(l, MINUS, _strdup("-"));
        else if (c == '*')
            emit_token(l, STAR, _strdup("*"));
        else if (c == '/')
            emit_token(l, SLASH, _strdup("/"));
        else if (c == '=')
            emit_token(l, EQUAL, _strdup("="));
        else if (c == ';')
            emit_token(l, SEMICOLON, _strdup(";"));
        else if (c == '(')
            emit_token(l, LEFT_PAREN, _strdup("("));
        else if (c == ')')
            emit_token(l, RIGHT_PAREN, _strdup(")"));
    }
    emit_token(l, END_OF_FILE, "");
}

char* tokenTypeToString(TokenType t) {
    switch (t) {
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case STAR: return "STAR";
        case SLASH: return "SLASH";
        case EQUAL: return "EQUAL";
        case SEMICOLON: return "SEMICOLON";
        case LEFT_PAREN: return "LEFT_PAREN";
        case RIGHT_PAREN: return "RIGHT_PAREN";
        case IDENTIFIER: return "IDENTIFIER";
        case NUMBER: return "NUMBER";
        case INT: return "INT";
        case END_OF_FILE: return "END_OF_FILE";
    }
    printf("unknown token\n");
    exit(1);
}

void delete_lexer(Lexer* l) {
    for (int i = 0; i < l->size - 1; i++) {
        free(l->tokens[i]->lexeme);
        free(l->tokens[i]);
    }
    int eof_idx = l->size - 1;
    free(l->tokens[eof_idx]);
    free(l->tokens);
    free(l->source);
    free(l);
}

int main() {
    
    char* code = read_file("dummy.c");

    Lexer* lexer = create_lexer(code);

    scan(lexer);

    for (int i = 0; i < lexer->size; i++)
        printf("[%s] %s\n", tokenTypeToString(lexer->tokens[i]->type), lexer->tokens[i]->lexeme);

    delete_lexer(lexer);
}
