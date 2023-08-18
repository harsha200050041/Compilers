#include "ast.hh"

extern int BLANK_SIZE;
extern std::string BLANK_STRING;

std::string binaryOpToString(binary_op op)
{
    switch (op)
    {
    case OR_OP:
        return "OR_OP";
    case AND_OP:
        return "AND_OP";
    case EQ_OP_:
        return "EQ_OP_";
    case NE_OP_:
        return "NE_OP_";
    case LT_OP_:
        return "LT_OP_";
    case GT_OP_:
        return "GT_OP_";
    case LE_OP_:
        return "LE_OP_";
    case GE_OP_:
        return "GE_OP_";
    case PLUS_:
        return "PLUS_";
    case MINUS_:
        return "MINUS_";
    case MULT_:
        return "MULT_";
    case DIV_:
        return "DIV_";
    default:
        return "UNKNOWN";
    }
}

std::string resolvedOpToString(resolved_op resolved)
{
    switch (resolved)
    {
    case _INT:
        return "INT";
    case _FLOAT:
        return "FLOAT";
    case _NA:
        return "";
    default:
        return "UNKNOWN";
    }
}

std::string unaryOpToString(unary_op op)
{
    switch (op)
    {
    case TO_FLOAT:
        return "TO_FLOAT";
    case TO_INT:
        return "TO_INT";
    case UMINUS:
        return "UMINUS";
    case NOT:
        return "NOT";
    case ADDRESS:
        return "ADDRESS";
    case DEREF:
        return "DEREF";
    case PP:
        return "PP";
    default:
        return "UNKNOWN";
    }
}

identifier_astnode::identifier_astnode(std::string exp_type, std::string name)
{
    this->astnode_type = IDENTIFIER_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = 1;
    this->name = name;
};

arrayref_astnode::arrayref_astnode(std::string exp_type, exp_astnode *array_exp, exp_astnode *index_exp)
{
    this->astnode_type = ARRAYREF_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = 1;
    this->array_exp = array_exp;
    this->index_exp = index_exp;
};

member_astnode::member_astnode(std::string exp_type, exp_astnode *struct_exp, identifier_astnode *field_identifier)
{
    this->astnode_type = MEMBER_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = struct_exp->l_value;
    this->struct_exp = struct_exp;
    this->field_identifier = field_identifier;
};

arrow_astnode::arrow_astnode(std::string exp_type, exp_astnode *pointer_exp, identifier_astnode *field_identifier)
{
    this->astnode_type = ARROW_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = 1;
    this->pointer_exp = pointer_exp;
    this->field_identifier = field_identifier;
};

op_binary_astnode::op_binary_astnode(std::string exp_type, binary_op op, resolved_op resolved, exp_astnode *left_exp, exp_astnode *right_exp)
{
    this->astnode_type = OP_BINARY_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = 0;
    this->op = op;
    this->resolved = resolved;
    this->left_exp = left_exp;
    this->right_exp = right_exp;
};

op_unary_astnode::op_unary_astnode(std::string exp_type, unary_op op, exp_astnode *child_exp)
{
    this->astnode_type = OP_UNARY_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = op == DEREF;
    this->op = op;
    this->child_exp = child_exp;
};

assignE_astnode::assignE_astnode(std::string exp_type, exp_astnode *left_exp, exp_astnode *right_exp)
{
    this->astnode_type = ASSIGNE_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = 0;
    this->left_exp = left_exp;
    this->right_exp = right_exp;
};

funcall_astnode::funcall_astnode(std::string exp_type, identifier_astnode *func_identifier, std::vector<exp_astnode *> param_exps)
{
    this->astnode_type = FUNCALL_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = 0;
    this->func_identifier = func_identifier;
    this->param_exps = param_exps;
};

intconst_astnode::intconst_astnode(std::string exp_type, int value)
{
    this->astnode_type = INTCONST_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = 0;
    this->value = value;
};

floatconst_astnode::floatconst_astnode(std::string exp_type, float value)
{
    this->astnode_type = FLOATCONST_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = 0;
    this->value = value;
};

stringconst_astnode::stringconst_astnode(std::string exp_type, std::string value)
{
    this->astnode_type = STRINGCONST_ASTNODE;
    this->exp_type = exp_type;
    this->l_value = 0;
    this->value = value;
};

empty_astnode::empty_astnode()
{
    this->astnode_type = EMPTY_ASTNODE;
    ;
};

seq_astnode::seq_astnode(std::vector<statement_astnode *> statements)
{
    this->astnode_type = SEQ_ASTNODE;
    this->statements = statements;
};

assignS_astnode::assignS_astnode(assignE_astnode *assign_exp)
{
    this->astnode_type = ASSIGNS_ASTNODE;
    this->left_exp = assign_exp->left_exp;
    this->right_exp = assign_exp->right_exp;
};

return_astnode::return_astnode(exp_astnode *return_exp)
{
    this->astnode_type = RETURN_ASTNODE;
    this->return_exp = return_exp;
};

if_astnode::if_astnode(exp_astnode *cond_exp, statement_astnode *then_statement, statement_astnode *else_statement)
{
    this->astnode_type = IF_ASTNODE;
    this->cond_exp = cond_exp;
    this->then_statement = then_statement;
    this->else_statement = else_statement;
};

while_astnode::while_astnode(exp_astnode *cond_exp, statement_astnode *while_statement)
{
    this->astnode_type = WHILE_ASTNODE;
    this->cond_exp = cond_exp;
    this->while_statement = while_statement;
};

for_astnode::for_astnode(exp_astnode *init_exp, exp_astnode *guard_exp, exp_astnode *step_exp, statement_astnode *for_statement)
{
    this->astnode_type = FOR_ASTNODE;
    this->init_exp = init_exp;
    this->guard_exp = guard_exp;
    this->step_exp = step_exp;
    this->for_statement = for_statement;
};

proccall_astnode::proccall_astnode(identifier_astnode *proc_identifier, std::vector<exp_astnode *> param_exps)
{
    this->astnode_type = PROCCALL_ASTNODE;
    this->proc_identifier = proc_identifier;
    this->param_exps = param_exps;
};

void identifier_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"identifier\": \"" << this->name << "\"\n"
              << blank_string << "}";
};

void arrayref_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"arrayref\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"array\": ";
    this->array_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"index\": ";
    this->index_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void member_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"member\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"struct\": ";
    this->struct_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"field\": ";
    this->field_identifier->print(blanks + 2 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void arrow_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"arrow\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"pointer\": ";
    this->pointer_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"field\": ";
    this->field_identifier->print(blanks + 2 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void op_binary_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"op_binary\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"op\": \"" << binaryOpToString(this->op) << resolvedOpToString(this->resolved) << "\",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"left\": ";
    this->left_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"right\": ";
    this->right_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void op_unary_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"op_unary\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"op\": \"" << unaryOpToString(this->op) << "\",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"child\": ";
    this->child_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void assignE_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"assignE\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"left\": ";
    this->left_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"right\": ";
    this->right_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void funcall_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"funcall\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"fname\": ";
    this->func_identifier->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n";
    if (this->param_exps.size())
    {
        std::cout << blank_string << BLANK_STRING << BLANK_STRING << "\"params\": [\n"
                  << blank_string << BLANK_STRING << BLANK_STRING << BLANK_STRING;
        for (uint i = 0; i < this->param_exps.size(); i++)
        {
            if (i != 0)
            {
                std::cout << ",\n"
                          << blank_string << BLANK_STRING << BLANK_STRING << BLANK_STRING;
            }
            this->param_exps[i]->print(blanks + 3 * BLANK_SIZE);
        }
        std::cout << "\n"
                  << blank_string << BLANK_STRING << BLANK_STRING << "]\n";
    }
    else
    {

        std::cout << blank_string << BLANK_STRING << BLANK_STRING << "\"params\": []\n";
    }
    std::cout << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void intconst_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"intconst\": " << this->value << "\n"
              << blank_string << "}";
};

void floatconst_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"floatconst\": " << this->value << "\n"
              << blank_string << "}";
};

void stringconst_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"stringconst\": " << this->value << "\n"
              << blank_string << "}";
};

void empty_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "\"empty\"";
};

void seq_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n";
    if (this->statements.size())
    {
        std::cout << blank_string << BLANK_STRING << "\"seq\": [\n"
                  << blank_string << BLANK_STRING << BLANK_STRING;
        for (uint i = 0; i < this->statements.size(); i++)
        {
            if (i != 0)
            {
                std::cout << ",\n"
                          << blank_string << BLANK_STRING << BLANK_STRING;
            }
            this->statements[i]->print(blanks + 2 * BLANK_SIZE);
        }
        std::cout << "\n"
                  << blank_string << BLANK_STRING << "]\n";
    }
    else
    {

        std::cout << blank_string << BLANK_STRING << "\"seq\": []\n";
    }
    std::cout << blank_string << "}";
};

void assignS_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"assignS\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"left\": ";
    this->left_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"right\": ";
    this->right_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void return_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"return\": ";
    this->return_exp->print(blanks + 1 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << "}";
};

void if_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"if\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"cond\": ";
    this->cond_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"then\": ";
    this->then_statement->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"else\": ";
    this->else_statement->print(blanks + 2 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void while_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"while\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"cond\": ";
    this->cond_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"stmt\": ";
    this->while_statement->print(blanks + 2 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void for_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"for\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"init\": ";
    this->init_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"guard\": ";
    this->guard_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"step\": ";
    this->step_exp->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"body\": ";
    this->for_statement->print(blanks + 2 * BLANK_SIZE);
    std::cout << "\n"
              << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};

void proccall_astnode::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    std::cout << "{\n"
              << blank_string << BLANK_STRING << "\"proccall\": {\n"
              << blank_string << BLANK_STRING << BLANK_STRING << "\"fname\": ";
    this->proc_identifier->print(blanks + 2 * BLANK_SIZE);
    std::cout << ",\n";
    if (this->param_exps.size())
    {
        std::cout << blank_string << BLANK_STRING << BLANK_STRING << "\"params\": [\n"
                  << blank_string << BLANK_STRING << BLANK_STRING << BLANK_STRING;
        for (uint i = 0; i < this->param_exps.size(); i++)
        {
            if (i != 0)
            {
                std::cout << ",\n"
                          << blank_string << BLANK_STRING << BLANK_STRING << BLANK_STRING;
            }
            this->param_exps[i]->print(blanks + 3 * BLANK_SIZE);
        }
        std::cout << "\n"
                  << blank_string << BLANK_STRING << BLANK_STRING << "]\n";
    }
    else
    {

        std::cout << blank_string << BLANK_STRING << BLANK_STRING << "\"params\": []\n";
    }
    std::cout << blank_string << BLANK_STRING << "}\n"
              << blank_string << "}";
};
