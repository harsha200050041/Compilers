#ifndef AST_H
#define AST_H
#include <bits/stdc++.h>

enum binary_op
{
    OR_OP,
    AND_OP,
    EQ_OP_,
    NE_OP_,
    LT_OP_,
    GT_OP_,
    LE_OP_,
    GE_OP_,
    PLUS_,
    MINUS_,
    MULT_,
    DIV_,
};

enum resolved_op
{
    _INT,
    _FLOAT,
    _NA
};

enum unary_op
{
    TO_FLOAT,
    TO_INT,
    UMINUS,
    NOT,
    ADDRESS,
    DEREF,
    PP,
};

enum node_type
{
    IDENTIFIER_ASTNODE,
    ARRAYREF_ASTNODE,
    MEMBER_ASTNODE,
    ARROW_ASTNODE,
    OP_BINARY_ASTNODE,
    OP_UNARY_ASTNODE,
    ASSIGNE_ASTNODE,
    FUNCALL_ASTNODE,
    INTCONST_ASTNODE,
    FLOATCONST_ASTNODE,
    STRINGCONST_ASTNODE,
    EMPTY_ASTNODE,
    SEQ_ASTNODE,
    ASSIGNS_ASTNODE,
    RETURN_ASTNODE,
    IF_ASTNODE,
    WHILE_ASTNODE,
    FOR_ASTNODE,
    PROCCALL_ASTNODE,
};

class abstract_astnode
{
public:
    enum node_type astnode_type;

    virtual void print(int blanks) = 0;
};

class statement_astnode : public abstract_astnode
{
};

class exp_astnode : public abstract_astnode
{
public:
    std::string exp_type;
    int l_value;
};

class ref_astnode : public exp_astnode
{
};

class identifier_astnode : public ref_astnode
{
public:
    std::string name;

    identifier_astnode(std::string exp_type, std::string name);
    void print(int blanks);
};

class arrayref_astnode : public ref_astnode
{
public:
    exp_astnode *array_exp, *index_exp;

    arrayref_astnode(std::string exp_type, exp_astnode *array_exp, exp_astnode *index_exp);
    void print(int blanks);
};

class member_astnode : public ref_astnode
{
public:
    exp_astnode *struct_exp;
    identifier_astnode *field_identifier;

    member_astnode(std::string exp_type, exp_astnode *struct_exp, identifier_astnode *field_identifier);
    void print(int blanks);
};

class arrow_astnode : public ref_astnode
{
public:
    exp_astnode *pointer_exp;
    identifier_astnode *field_identifier;

    arrow_astnode(std::string exp_type, exp_astnode *pointer_exp, identifier_astnode *field_identifier);
    void print(int blanks);
};

class op_binary_astnode : public exp_astnode
{
public:
    binary_op op;
    resolved_op resolved;
    exp_astnode *left_exp, *right_exp;

    op_binary_astnode(std::string exp_type, binary_op op, resolved_op resolved, exp_astnode *left_exp, exp_astnode *right_exp);
    void print(int blanks);
};

class op_unary_astnode : public exp_astnode
{
public:
    unary_op op;
    exp_astnode *child_exp;

    op_unary_astnode(std::string exp_type, unary_op op, exp_astnode *child_exp);
    void print(int blanks);
};

class assignE_astnode : public exp_astnode
{
public:
    exp_astnode *left_exp, *right_exp;

    assignE_astnode(std::string exp_type, exp_astnode *left_exp, exp_astnode *right_exp);
    void print(int blanks);
};

class funcall_astnode : public exp_astnode
{
public:
    identifier_astnode *func_identifier;
    std::vector<exp_astnode *> param_exps;

    funcall_astnode(std::string exp_type, identifier_astnode *func_identifier, std::vector<exp_astnode *> param_exps);
    void print(int blanks);
};

class intconst_astnode : public exp_astnode
{
public:
    int value;

    intconst_astnode(std::string exp_type, int value);
    void print(int blanks);
};

class floatconst_astnode : public exp_astnode
{
public:
    float value;

    floatconst_astnode(std::string exp_type, float value);
    void print(int blanks);
};

class stringconst_astnode : public exp_astnode
{
public:
    std::string value;

    stringconst_astnode(std::string exp_type, std::string value);
    void print(int blanks);
};

class empty_astnode : public statement_astnode
{
public:
    // No children

    empty_astnode();
    void print(int blanks);
};

class seq_astnode : public statement_astnode
{
public:
    std::vector<statement_astnode *> statements;

    seq_astnode(std::vector<statement_astnode *> statements);
    void print(int blanks);
};

class assignS_astnode : public statement_astnode
{
public:
    exp_astnode *left_exp, *right_exp;

    assignS_astnode(assignE_astnode *assign_exp);
    void print(int blanks);
};

class return_astnode : public statement_astnode
{
public:
    exp_astnode *return_exp;

    return_astnode(exp_astnode *return_exp);
    void print(int blanks);
};

class if_astnode : public statement_astnode
{
public:
    exp_astnode *cond_exp;
    statement_astnode *then_statement, *else_statement;

    if_astnode(exp_astnode *cond_exp, statement_astnode *then_statement, statement_astnode *else_statement);
    void print(int blanks);
};

class while_astnode : public statement_astnode
{
public:
    exp_astnode *cond_exp;
    statement_astnode *while_statement;

    while_astnode(exp_astnode *cond_exp, statement_astnode *while_statement);
    void print(int blanks);
};

class for_astnode : public statement_astnode
{
public:
    exp_astnode *init_exp, *guard_exp, *step_exp;
    statement_astnode *for_statement;

    for_astnode(exp_astnode *init_exp, exp_astnode *guard_exp, exp_astnode *step_exp, statement_astnode *for_statement);
    void print(int blanks);
};

class proccall_astnode : public statement_astnode
{
public:
    identifier_astnode *proc_identifier;
    std::vector<exp_astnode *> param_exps;

    proccall_astnode(identifier_astnode *func_identifier, std::vector<exp_astnode *> param_exps);
    void print(int blanks);
};

#endif