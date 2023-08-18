#include "asm.hh"

const int REGISTER_SIZE = 4;
const int MINI_REGISTER = 2;

const std::string EBP = "%ebp";
const std::string ESP = "%esp";
const std::string EAX = "%eax";
const std::string ECX = "%ecx";
const std::string EDX = "%edx";
const std::string EBX = "%ebx";
const std::string ESI = "%esi";
const std::string EDI = "%edi";
const std::string ADD_INST = "addl";
const std::string SUB_INST = "subl";
const std::string MUL_INST = "imull";
const std::string DIV_INST = "idivl";
const int NUM_REGISTERS = 6;
const std::vector<std::string> REGISTERS({EAX, ECX, EDX, EBX, ESI, EDI});

int con_label = 0;
int jmp_label = 2;
std::stack<std::string> registers;

SymbTab *symbTab;
std::vector<std::string> instructions;
std::vector<int> returnlist;
int stack_size;
std::string curr_func_name;

extern std::map<std::string, abstract_astnode *> ast;
extern SymbTab *gst;
extern std::map<std::string, std::vector<decl_struct *>> func_params;

int size(std::string type)
{
    if (type == INT_TYPE)
        return INT_SIZE;
    if (type == FLOAT_TYPE)
        return FLOAT_SIZE;
    if (isPointer(type))
        return ADDRESS_SIZE;
    if (isArray(type))
        return std::stoi(type.substr(type.find("[") + 1, type.find("]") - type.find("["))) * size(deref(type));
    if (isStruct(type))
        return gst->entries[type]->size;
    return ADDRESS_SIZE;
}

std::string binaryOpToInst(binary_op op)
{
    switch (op)
    {
    case PLUS_:
        return "addl";
    case MINUS_:
        return "subl";
    case MULT_:
        return "imull";
    case DIV_:
        return "idivl";
    default:
        return "UNKNOWN";
    }
}

std::string gen_con_label(std::string con_string)
{
    std::cout << "\t.text\n"
              << "\t.section\t.rodata\n"
              << ".LC" << con_label << ":\n"
              << "\t.string\t" << con_string << "\n";
    return ".LC" + std::to_string(con_label++);
};

std::string gen_jmp_label()
{
    return ".L" + std::to_string(jmp_label++);
}

void jmp_uncond(std::string label = "")
{
    instructions.push_back("\tjmp\t" + label);
};

void jmp_eq(std::string reg1, std::string reg2, std::string label = "")
{
    instructions.push_back("\tcmpl\t" + reg1 + ", " + reg2 + "\n\tje\t" + label);
};

void jmp_ne(std::string reg1, std::string reg2, std::string label = "")
{
    instructions.push_back("\tcmpl\t" + reg1 + ", " + reg2 + "\n\tjne\t" + label);
};

void jmp_eq_zero(std::string reg, std::string label = "")
{
    instructions.push_back("\tcmpl\t$0, " + reg + "\n\tje\t" + label);
};

void jmp_ne_zero(std::string reg, std::string label = "")
{
    instructions.push_back("\tcmpl\t$0, " + reg + "\n\tjne\t" + label);
};

void jmp_lt(std::string reg1, std::string reg2, std::string label = "")
{
    instructions.push_back("\tcmpl\t" + reg1 + ", " + reg2 + "\n\tjl\t" + label);
};

void jmp_gt(std::string reg1, std::string reg2, std::string label = "")
{
    instructions.push_back("\tcmpl\t" + reg1 + ", " + reg2 + "\n\tjg\t" + label);
};

void jmp_le(std::string reg1, std::string reg2, std::string label = "")
{
    instructions.push_back("\tcmpl\t" + reg1 + ", " + reg2 + "\n\tjle\t" + label);
};

void jmp_ge(std::string reg1, std::string reg2, std::string label = "")
{
    instructions.push_back("\tcmpl\t" + reg1 + ", " + reg2 + "\n\tjge\t" + label);
};

void mov_reg_reg(std::string reg1, std::string reg2, int disp = 0)
{
    if (disp)
        instructions.push_back("\tleal\t" + std::to_string(disp) + "(" + reg1 + "), " + reg2);
    else
        instructions.push_back("\tmovl\t" + reg1 + ", " + reg2);
    return;
};

void mov_reg_mem(std::string reg, std::string mem, int disp = 0)
{
    if (disp)
        instructions.push_back("\tmovl\t" + reg + ", " + std::to_string(disp) + "(" + mem + ")");
    else
        instructions.push_back("\tmovl\t" + reg + ", (" + mem + ")");
    return;
};

void mov_mem_reg(std::string mem, std::string reg, int disp = 0)
{
    if (disp)
        instructions.push_back("\tmovl\t" + std::to_string(disp) + "(" + mem + "), " + reg);
    else
        instructions.push_back("\tmovl\t(" + mem + "), " + reg);
    return;
};

void mov_con_reg(std::string con, std::string reg)
{
    instructions.push_back("\tmovl\t$" + con + ", " + reg);
    return;
};

void mov_con_mem(std::string con, std::string mem, int disp = 0)
{
    if (disp)
        instructions.push_back("\tmovl\t$" + con + ", " + std::to_string(disp) + "(" + mem + ")");
    else
        instructions.push_back("\tmovl\t$" + con + ", (" + mem + ")");
    return;
};

void pop_reg(std::string reg)
{
    instructions.push_back("\tpopl\t" + reg);
    return;
};

void push_reg(std::string reg)
{
    instructions.push_back("\tpushl\t" + reg);
    return;
};

void push_mem(std::string mem, int disp = 0)
{
    if (disp)
        instructions.push_back("\tpushl\t" + std::to_string(disp) + "(" + mem + ")");
    else
        instructions.push_back("\tpushl\t(" + mem + ")");
    return;
};

void push_con(std::string con)
{
    instructions.push_back("\tpushl\t$" + con);
    return;
};

void call_fun(std::string func_name)
{
    instructions.push_back("\tcall\t" + func_name);
    return;
};

void neg_reg(std::string reg)
{
    instructions.push_back("\tnegl\t" + reg);
    return;
};

void op_reg_reg(binary_op op, std::string reg1, std::string reg2)
{
    instructions.push_back("\t" + binaryOpToInst(op) + "\t" + reg1 + ", " + reg2);
    return;
};

void op_con_reg(binary_op op, std::string con, std::string reg)
{
    instructions.push_back("\t" + binaryOpToInst(op) + "\t$" + con + ", " + reg);
    return;
};

void gen_func_prolog(std::string func_name)
{
    std::cout << "\t.text\n"
              << "\t.globl\t" << func_name << "\n"
              << "\t.type\t" << func_name << ", @function\n"
              << func_name << ":\n";
    return;
};

void gen_func_epilog(std::string func_name)
{
    std::cout << "\tleave\n"
              << "\tret\n"
              << "\t.size\t" << func_name << ", .-" << func_name << "\n";
    return;
};

void gen_node_rodata(abstract_astnode *node)
{
    if (node->astnode_type == SEQ_ASTNODE)
    {
        seq_astnode *seq_node = (seq_astnode *)node;
        for (uint i = 0; i < seq_node->statements.size(); i++)
        {
            gen_node_rodata(seq_node->statements[i]);
        };
    }
    else if (node->astnode_type == IF_ASTNODE)
    {
        if_astnode *if_node = (if_astnode *)node;
        gen_node_rodata(if_node->then_statement);
        gen_node_rodata(if_node->else_statement);
    }
    else if (node->astnode_type == WHILE_ASTNODE)
    {
        while_astnode *while_node = (while_astnode *)node;
        gen_node_rodata(while_node->while_statement);
    }
    else if (node->astnode_type == FOR_ASTNODE)
    {
        for_astnode *for_node = (for_astnode *)node;
        gen_node_rodata(for_node->for_statement);
    }
    else if (node->astnode_type == PROCCALL_ASTNODE)
    {
        proccall_astnode *proccall_node = (proccall_astnode *)node;
        if (proccall_node->param_exps.size() && proccall_node->param_exps[0]->astnode_type == STRINGCONST_ASTNODE)
        {
            stringconst_astnode *stringconst_node = (stringconst_astnode *)proccall_node->param_exps[0];
            stringconst_node->value = gen_con_label(stringconst_node->value);
        }
    }
    return;
};

abstract_astnode *apply_constant_folding(abstract_astnode *node)
{
    if (node->astnode_type == ARRAYREF_ASTNODE)
    {
        arrayref_astnode *arrayref_node = (arrayref_astnode *)node;
        arrayref_node->array_exp = (exp_astnode *)apply_constant_folding(arrayref_node->array_exp);
        arrayref_node->index_exp = (exp_astnode *)apply_constant_folding(arrayref_node->index_exp);
    }
    else if (node->astnode_type == MEMBER_ASTNODE)
    {
        member_astnode *member_node = (member_astnode *)node;
        member_node->struct_exp = (exp_astnode *)apply_constant_folding(member_node->struct_exp);
    }
    else if (node->astnode_type == ARROW_ASTNODE)
    {
        arrow_astnode *arrow_node = (arrow_astnode *)node;
        arrow_node->pointer_exp = (exp_astnode *)apply_constant_folding(arrow_node->pointer_exp);
    }
    else if (node->astnode_type == OP_BINARY_ASTNODE)
    {
        op_binary_astnode *op_binary_node = (op_binary_astnode *)node;
        op_binary_node->left_exp = (exp_astnode *)apply_constant_folding(op_binary_node->left_exp);
        op_binary_node->right_exp = (exp_astnode *)apply_constant_folding(op_binary_node->right_exp);
        if (op_binary_node->left_exp->astnode_type == INTCONST_ASTNODE && op_binary_node->right_exp->astnode_type == INTCONST_ASTNODE)
        {
            int left_value = ((intconst_astnode *)(op_binary_node->left_exp))->value;
            int right_value = ((intconst_astnode *)(op_binary_node->right_exp))->value;
            if (op_binary_node->op == OR_OP)
            {
                return new intconst_astnode(INT_TYPE, left_value || right_value);
            }
            else if (op_binary_node->op == AND_OP)
            {
                return new intconst_astnode(INT_TYPE, left_value && right_value);
            }
            else if (op_binary_node->op == EQ_OP_)
            {
                return new intconst_astnode(INT_TYPE, left_value == right_value);
            }
            else if (op_binary_node->op == NE_OP_)
            {
                return new intconst_astnode(INT_TYPE, left_value != right_value);
            }
            else if (op_binary_node->op == LT_OP_)
            {
                return new intconst_astnode(INT_TYPE, left_value < right_value);
            }
            else if (op_binary_node->op == GT_OP_)
            {
                return new intconst_astnode(INT_TYPE, left_value > right_value);
            }
            else if (op_binary_node->op == LE_OP_)
            {
                return new intconst_astnode(INT_TYPE, left_value <= right_value);
            }
            else if (op_binary_node->op == GE_OP_)
            {
                return new intconst_astnode(INT_TYPE, left_value >= right_value);
            }
            else if (op_binary_node->op == PLUS_)
            {
                return new intconst_astnode(INT_TYPE, left_value + right_value);
            }
            else if (op_binary_node->op == MINUS_)
            {
                return new intconst_astnode(INT_TYPE, left_value - right_value);
            }
            else if (op_binary_node->op == MULT_)
            {
                return new intconst_astnode(INT_TYPE, left_value * right_value);
            }
            else if (op_binary_node->op == DIV_)
            {
                return new intconst_astnode(INT_TYPE, left_value / right_value);
            }
        }
    }
    else if (node->astnode_type == OP_UNARY_ASTNODE)
    {
        op_unary_astnode *op_unary_node = (op_unary_astnode *)node;
        int child_value = ((intconst_astnode *)(op_unary_node->child_exp))->value;
        if (op_unary_node->child_exp->astnode_type == INTCONST_ASTNODE)
        {
            intconst_astnode *child_node = (intconst_astnode *)(op_unary_node->child_exp);
            if (op_unary_node->op == UMINUS)
            {
                return new intconst_astnode(INT_TYPE, -child_value);
            }
            else if (op_unary_node->op == PP)
            {
                return new intconst_astnode(INT_TYPE, child_value++);
            }
            else if (op_unary_node->op == NOT)
            {
                return new intconst_astnode(INT_TYPE, !child_value);
            }
        }
    }
    else if (node->astnode_type == ASSIGNE_ASTNODE)
    {
        assignE_astnode *assignE_node = (assignE_astnode *)node;
        assignE_node->left_exp = (exp_astnode *)apply_constant_folding(assignE_node->left_exp);
        assignE_node->right_exp = (exp_astnode *)apply_constant_folding(assignE_node->right_exp);
    }
    else if (node->astnode_type == FUNCALL_ASTNODE)
    {
        funcall_astnode *funcall_node = (funcall_astnode *)node;
        for (uint i = 0; i < funcall_node->param_exps.size(); i++)
        {
            funcall_node->param_exps[i] = (exp_astnode *)apply_constant_folding(funcall_node->param_exps[i]);
        };
    }
    else if (node->astnode_type == SEQ_ASTNODE)
    {
        seq_astnode *seq_node = (seq_astnode *)node;
        for (uint i = 0; i < seq_node->statements.size(); i++)
        {
            apply_constant_folding(seq_node->statements[i]);
        };
    }
    else if (node->astnode_type == ASSIGNS_ASTNODE)
    {
        assignS_astnode *assignS_node = (assignS_astnode *)node;
        assignS_node->left_exp = (exp_astnode *)apply_constant_folding(assignS_node->left_exp);
        assignS_node->right_exp = (exp_astnode *)apply_constant_folding(assignS_node->right_exp);
    }
    else if (node->astnode_type == RETURN_ASTNODE)
    {
        return_astnode *return_node = (return_astnode *)node;
        return_node->return_exp = (exp_astnode *)apply_constant_folding(return_node->return_exp);
    }
    else if (node->astnode_type == IF_ASTNODE)
    {
        if_astnode *if_node = (if_astnode *)node;
        if_node->cond_exp = (exp_astnode *)apply_constant_folding(if_node->cond_exp);
        apply_constant_folding(if_node->then_statement);
        apply_constant_folding(if_node->else_statement);
    }
    else if (node->astnode_type == WHILE_ASTNODE)
    {
        while_astnode *while_node = (while_astnode *)node;
        while_node->cond_exp = (exp_astnode *)apply_constant_folding(while_node->cond_exp);
        apply_constant_folding(while_node->while_statement);
    }
    else if (node->astnode_type == FOR_ASTNODE)
    {
        for_astnode *for_node = (for_astnode *)node;
        for_node->init_exp = (exp_astnode *)apply_constant_folding(for_node->init_exp);
        for_node->guard_exp = (exp_astnode *)apply_constant_folding(for_node->guard_exp);
        for_node->step_exp = (exp_astnode *)apply_constant_folding(for_node->step_exp);
        apply_constant_folding(for_node->for_statement);
    }
    else if (node->astnode_type == PROCCALL_ASTNODE)
    {
        proccall_astnode *proccall_node = (proccall_astnode *)node;
        for (uint i = 0; i < proccall_node->param_exps.size(); i++)
        {
            proccall_node->param_exps[i] = (exp_astnode *)apply_constant_folding(proccall_node->param_exps[i]);
        };
    }
    return node;
}

void save_reg(std::string reg)
{
    op_con_reg(MINUS_, std::to_string(REGISTER_SIZE), ESP);
    push_reg(reg);
    registers.push(reg);
};

std::string restore_reg(std::string reg = "")
{
    if (reg == "")
    {
        pop_reg(registers.top());
        return registers.top();
    }
    else
    {
        pop_reg(reg);
        return reg;
    }
}

int nextInstr()
{
    return instructions.size();
}

void backpatch(std::vector<int> list, std::string target)
{
    for (auto e : list)
    {
        instructions[e] += target;
    }
}

std::vector<int> merge(std::vector<int> list1, std::vector<int> list2)
{
    std::vector<int> merged;
    merged.reserve(list1.size() + list2.size());
    merged.insert(merged.end(), list1.begin(), list1.end());
    merged.insert(merged.end(), list2.begin(), list2.end());
    return merged;
}

struct attributes gen_or(op_binary_astnode *node, int fall)
{
    struct attributes or_attr;
    struct attributes left_attr = gen_boolean(node->left_exp, 0);
    if (left_attr.falselist.size())
    {
        std::string left_false_label = gen_jmp_label();
        backpatch(left_attr.falselist, left_false_label);
        instructions.push_back(left_false_label + ":");
    }
    struct attributes right_attr = gen_boolean(node->right_exp, fall);
    or_attr.truelist = merge(left_attr.truelist, right_attr.truelist);
    or_attr.falselist = right_attr.falselist;
    return or_attr;
}

struct attributes gen_and(op_binary_astnode *node, int fall)
{
    struct attributes and_attr;
    struct attributes left_attr = gen_boolean(node->left_exp, 1);
    if (left_attr.truelist.size())
    {
        std::string left_true_label = gen_jmp_label();
        backpatch(left_attr.truelist, left_true_label);
        instructions.push_back(left_true_label + ":");
    }
    struct attributes right_attr = gen_boolean(node->right_exp, fall);
    and_attr.falselist = merge(left_attr.falselist, right_attr.falselist);
    and_attr.truelist = right_attr.truelist;
    return and_attr;
}

struct attributes gen_rel(op_binary_astnode *node, int fall)
{
    struct attributes eq_attr;
    struct attributes left_attr = gen(node->left_exp, 1);
    std::string left_reg = registers.top();
    registers.pop();
    struct attributes right_attr = gen(node->right_exp, 1);
    std::string right_reg = registers.top();
    registers.push(left_reg);

    if (fall)
    {
        eq_attr.falselist.push_back(nextInstr());
        switch (node->op)
        {
        case EQ_OP_:
            jmp_ne(right_reg, left_reg);
            return eq_attr;
        case NE_OP_:
            jmp_eq(right_reg, left_reg);
            return eq_attr;
        case LT_OP_:
            jmp_ge(right_reg, left_reg);
            return eq_attr;
        case GT_OP_:
            jmp_le(right_reg, left_reg);
            return eq_attr;
        case LE_OP_:
            jmp_gt(right_reg, left_reg);
            return eq_attr;
        case GE_OP_:
            jmp_lt(right_reg, left_reg);
            return eq_attr;
        default:
            eq_attr.falselist.pop_back();
            return eq_attr;
        }
    }
    else
    {
        eq_attr.truelist.push_back(nextInstr());
        switch (node->op)
        {
        case EQ_OP_:
            jmp_eq(right_reg, left_reg);
            return eq_attr;
        case NE_OP_:
            jmp_ne(right_reg, left_reg);
            return eq_attr;
        case LT_OP_:
            jmp_lt(right_reg, left_reg);
            return eq_attr;
        case GT_OP_:
            jmp_gt(right_reg, left_reg);
            return eq_attr;
        case LE_OP_:
            jmp_le(right_reg, left_reg);
            return eq_attr;
        case GE_OP_:
            jmp_ge(right_reg, left_reg);
            return eq_attr;
        default:
            eq_attr.truelist.pop_back();
            return eq_attr;
        }
    }
    return eq_attr;
}

struct attributes gen_not(op_unary_astnode *node, int fall)
{
    struct attributes not_attr;
    struct attributes child_attr = gen_boolean(node->child_exp, !fall);
    not_attr.truelist = child_attr.falselist;
    not_attr.falselist = child_attr.truelist;
    return not_attr;
}

struct attributes gen_boolean(abstract_astnode *node, int fall)
{
    switch (node->astnode_type)
    {
    case OP_BINARY_ASTNODE:
        switch (((op_binary_astnode *)node)->op)
        {
        case OR_OP:
            return gen_or((op_binary_astnode *)node, fall);
        case AND_OP:
            return gen_and((op_binary_astnode *)node, fall);
        case EQ_OP_:
        case NE_OP_:
        case LT_OP_:
        case GT_OP_:
        case LE_OP_:
        case GE_OP_:
            return gen_rel((op_binary_astnode *)node, fall);
        default:
            break;
        }
    case OP_UNARY_ASTNODE:
        switch (((op_unary_astnode *)node)->op)
        {
        case NOT:
            return gen_not((op_unary_astnode *)node, fall);
        default:
            break;
        }
    default:
    {
        gen(node, 1);
        struct attributes attr;
        if (fall)
        {
            attr.falselist.push_back(nextInstr());
            jmp_eq_zero(registers.top());
        }
        else
        {
            attr.truelist.push_back(nextInstr());
            jmp_ne_zero(registers.top());
        }
        return attr;
    }
    }
}

struct attributes gen_identifier(identifier_astnode *node, int value)
{
    struct attributes identifier_attr;
    if (value)
    {
        if (isArray(node->exp_type) || isStruct(node->exp_type))
            mov_reg_reg(EBP, registers.top(), symbTab->entries[node->name]->offset);
        else
            mov_mem_reg(EBP, registers.top(), symbTab->entries[node->name]->offset);
    }
    else
    {
        if (symbTab->entries[node->name]->offset > 0)
        {
            if (isArray(symbTab->entries[node->name]->type))
            {
                mov_mem_reg(EBP, registers.top(), symbTab->entries[node->name]->offset);
            }
            else
            {
                mov_reg_reg(EBP, registers.top(), symbTab->entries[node->name]->offset);
            }
        }
        else
            mov_reg_reg(EBP, registers.top(), symbTab->entries[node->name]->offset);
    }
    return identifier_attr;
};

struct attributes gen_arrayref(arrayref_astnode *node, int value)
{
    struct attributes arrayref_attr;
    value = value && !isArray(node->exp_type);
    int offset = 0;
    while (node->array_exp->astnode_type == ARRAYREF_ASTNODE && node->index_exp->astnode_type == INTCONST_ASTNODE)
    {
        offset += ((intconst_astnode *)node->index_exp)->value * size(deref(node->array_exp->exp_type));
        node = (arrayref_astnode *)node->array_exp;
    }
    if (isPointer(node->array_exp->exp_type))
        gen(node->array_exp, 1);
    else
        gen(node->array_exp, 0);
    if (node->index_exp->astnode_type == INTCONST_ASTNODE)
    {
        offset += ((intconst_astnode *)node->index_exp)->value * size(deref(node->array_exp->exp_type));
        op_con_reg(PLUS_, std::to_string(offset), registers.top());
    }
    else
    {
        std::string array_reg = registers.top();
        registers.pop();
        gen(node->index_exp, 1);
        op_con_reg(MULT_, std::to_string(size(deref(node->array_exp->exp_type))), registers.top());
        op_reg_reg(PLUS_, registers.top(), array_reg);
        registers.push(array_reg);
        op_con_reg(PLUS_, std::to_string(offset), registers.top());
    }
    if (value)
        mov_mem_reg(registers.top(), registers.top());
    return arrayref_attr;
};

struct attributes gen_member(member_astnode *node, int value)
{
    struct attributes member_attr;
    gen(node->struct_exp, 0);
    if (value)
        mov_mem_reg(registers.top(), registers.top(), gst->entries[node->struct_exp->exp_type]->symbtab->entries[node->field_identifier->name]->offset);
    else if (gst->entries[node->struct_exp->exp_type]->symbtab->entries[node->field_identifier->name]->offset)
        mov_reg_reg(registers.top(), registers.top(), gst->entries[node->struct_exp->exp_type]->symbtab->entries[node->field_identifier->name]->offset);
    return member_attr;
};

struct attributes gen_arrow(arrow_astnode *node, int value)
{
    struct attributes arrow_attr;
    gen(node->pointer_exp, 0);
    if (!isArray(node->pointer_exp->exp_type))
        mov_mem_reg(registers.top(), registers.top());
    if (value)
        mov_mem_reg(registers.top(), registers.top(), gst->entries[deref(node->pointer_exp->exp_type)]->symbtab->entries[node->field_identifier->name]->offset);
    else if (gst->entries[deref(node->pointer_exp->exp_type)]->symbtab->entries[node->field_identifier->name]->offset)
        mov_reg_reg(registers.top(), registers.top(), gst->entries[deref(node->pointer_exp->exp_type)]->symbtab->entries[node->field_identifier->name]->offset);
    return arrow_attr;
};

struct attributes gen_op_binary(op_binary_astnode *node, int value)
{
    struct attributes op_binary_attr;

    switch (node->op)
    {
    case OR_OP:
    case AND_OP:
    case EQ_OP_:
    case NE_OP_:
    case LT_OP_:
    case GT_OP_:
    case LE_OP_:
    case GE_OP_:
    {
        struct attributes attr = gen_boolean(node, 1);
        if (attr.truelist.size())
        {
            std::string true_label = gen_jmp_label();
            backpatch(attr.truelist, true_label);
            instructions.push_back(true_label + ":");
        }
        mov_con_reg("1", registers.top());
        std::string next_label = gen_jmp_label();
        jmp_uncond(next_label);
        if (attr.falselist.size())
        {
            std::string false_label = gen_jmp_label();
            backpatch(attr.falselist, false_label);
            instructions.push_back(false_label + ":");
        }
        mov_con_reg("0", registers.top());
        instructions.push_back(next_label + ":");
        return op_binary_attr;
    }
    case PLUS_:
    {
        if (node->right_exp->astnode_type == INTCONST_ASTNODE)
        {
            gen(node->left_exp, 1);
            int right_value = ((intconst_astnode *)node->right_exp)->value;
            if (isPointer(node->left_exp->exp_type) || isArray(node->left_exp->exp_type))
            {
                right_value *= size(deref(node->left_exp->exp_type));
            }
            op_con_reg(node->op, std::to_string(right_value), registers.top());
        }
        else if (node->left_exp->astnode_type == INTCONST_ASTNODE)
        {
            int left_value = ((intconst_astnode *)node->left_exp)->value;
            if (isPointer(node->right_exp->exp_type) || isArray(node->right_exp->exp_type))
            {
                left_value *= size(deref(node->right_exp->exp_type));
            }
            gen(node->right_exp, 1);
            op_con_reg(node->op, std::to_string(left_value), registers.top());
        }
        else
        {
            std::string left_reg, right_reg;
            gen(node->left_exp, 1);
            if (isPointer(node->right_exp->exp_type) || isArray(node->right_exp->exp_type))
            {
                op_con_reg(MULT_, std::to_string(size(deref(node->right_exp->exp_type))), registers.top());
            }
            left_reg = registers.top();
            registers.pop();
            if (registers.size() == MINI_REGISTER - 1)
                save_reg(left_reg);
            gen(node->right_exp, 1);
            if (isPointer(node->left_exp->exp_type) || isArray(node->left_exp->exp_type))
            {
                op_con_reg(MULT_, std::to_string(size(deref(node->left_exp->exp_type))), registers.top());
            }
            if (registers.size() == MINI_REGISTER - 1)
            {
                right_reg = registers.top();
                registers.pop();
                left_reg = restore_reg();
                registers.push(right_reg);
            }
            op_reg_reg(node->op, registers.top(), left_reg);
            registers.push(left_reg);
        }
        return op_binary_attr;
    }
    case MINUS_:
    {
        if (node->right_exp->astnode_type == INTCONST_ASTNODE)
        {
            gen(node->left_exp, 1);
            int right_value = ((intconst_astnode *)node->right_exp)->value;
            if (isPointer(node->left_exp->exp_type) || isArray(node->left_exp->exp_type))
            {
                right_value *= size(deref(node->left_exp->exp_type));
            }
            op_con_reg(node->op, std::to_string(right_value), registers.top());
        }
        else if (node->left_exp->astnode_type == INTCONST_ASTNODE)
        {
            int left_value = ((intconst_astnode *)node->left_exp)->value;
            gen(node->right_exp, 1);
            op_con_reg(node->op, std::to_string(left_value), registers.top());
        }
        else
        {
            std::string left_reg, right_reg;
            gen(node->left_exp, 1);
            left_reg = registers.top();
            registers.pop();
            if (registers.size() == MINI_REGISTER - 1)
                save_reg(left_reg);
            gen(node->right_exp, 1);
            if (isPointer(node->left_exp->exp_type) || isArray(node->left_exp->exp_type))
            {
                op_con_reg(MULT_, std::to_string(size(deref(node->left_exp->exp_type))), registers.top());
            }
            if (registers.size() == MINI_REGISTER - 1)
            {
                right_reg = registers.top();
                registers.pop();
                left_reg = restore_reg();
                registers.push(right_reg);
            }
            op_reg_reg(node->op, registers.top(), left_reg);
            registers.push(left_reg);
        }
        return op_binary_attr;
    }
    case MULT_:
    {
        if (node->right_exp->astnode_type == INTCONST_ASTNODE)
        {
            gen(node->left_exp, 1);
            int right_value = ((intconst_astnode *)node->right_exp)->value;
            op_con_reg(node->op, std::to_string(right_value), registers.top());
        }
        else if (node->left_exp->astnode_type == INTCONST_ASTNODE)
        {
            int left_value = ((intconst_astnode *)node->left_exp)->value;
            gen(node->right_exp, 1);
            op_con_reg(node->op, std::to_string(left_value), registers.top());
        }
        else
        {
            std::string left_reg, right_reg;
            gen(node->left_exp, 1);
            left_reg = registers.top();
            registers.pop();
            if (registers.size() == MINI_REGISTER - 1)
                save_reg(left_reg);
            gen(node->right_exp, 1);
            if (registers.size() == MINI_REGISTER - 1)
            {
                right_reg = registers.top();
                registers.pop();
                left_reg = restore_reg();
                registers.push(right_reg);
            }
            op_reg_reg(node->op, registers.top(), left_reg);
            registers.push(left_reg);
        }
        return op_binary_attr;
    }
    case DIV_:
    {
        int eax_saved = 0, edx_popped = 0;
        if (registers.top() != EAX)
        {
            eax_saved = 1;
            save_reg(EAX);
        }
        gen(node->left_exp, 1);
        registers.pop();
        if (registers.top() == EDX)
        {
            edx_popped = 1;
            registers.pop();
        }
        gen(node->right_exp, 1);
        instructions.push_back("\tcltd\n\t" + DIV_INST + "\t" + registers.top());
        if (edx_popped)
            registers.push(EDX);
        registers.push(EAX);
        if (eax_saved)
        {
            registers.pop();
            mov_reg_reg(EAX, registers.top());
            restore_reg(EAX);
        }
        return op_binary_attr;
    }
    default:
        return op_binary_attr;
    }
};

struct attributes gen_op_unary(op_unary_astnode *node, int value)
{
    struct attributes op_unary_attr;
    switch (node->op)
    {
    case UMINUS:
    {
        gen(node->child_exp, 1);
        neg_reg(registers.top());
        return op_unary_attr;
    }
    case NOT:
    {
        struct attributes attr = gen_boolean(node, 1);
        if (attr.truelist.size())
        {
            std::string true_label = gen_jmp_label();
            backpatch(attr.truelist, true_label);
            instructions.push_back(true_label + ":");
        }
        mov_con_reg("1", registers.top());
        std::string next_label = gen_jmp_label();
        jmp_uncond(next_label);
        if (attr.falselist.size())
        {
            std::string false_label = gen_jmp_label();
            backpatch(attr.falselist, false_label);
            instructions.push_back(false_label + ":");
        }
        mov_con_reg("0", registers.top());
        instructions.push_back(next_label + ":");
        return op_unary_attr;
    }
    case ADDRESS:
    {
        gen(node->child_exp, 0);
        return op_unary_attr;
    }
    case DEREF:
    {
        gen(node->child_exp, 1);
        if (value)
            mov_mem_reg(registers.top(), registers.top());
        return op_unary_attr;
    }
    case PP:
    {
        gen(node->child_exp, 0);
        std::string addr_reg = registers.top();
        registers.pop();
        mov_mem_reg(addr_reg, registers.top());
        if (isPointer(node->child_exp->exp_type) || isArray(node->child_exp->exp_type))
        {
            op_con_reg(PLUS_, std::to_string(size(deref(node->child_exp->exp_type))), registers.top());
        }
        else
        {
            op_con_reg(PLUS_, "1", registers.top());
        }
        mov_reg_mem(registers.top(), addr_reg);
        mov_reg_reg(registers.top(), addr_reg, -1);
        registers.push(addr_reg);
        return op_unary_attr;
    }
    default:
        return op_unary_attr;
    }
};

struct attributes gen_assignE(assignE_astnode *node, int value)
{
    struct attributes assignE_attr;
    std::string left_reg, right_reg;

    if (isStruct(node->right_exp->exp_type))
    {
        gen(node->right_exp, 0);
        right_reg = registers.top();
        registers.pop();
        gen(node->left_exp, 0);
        left_reg = registers.top();
        registers.pop();
        int offset = gst->entries[node->right_exp->exp_type]->size - REGISTER_SIZE;
        for (int j = 0; j < gst->entries[node->right_exp->exp_type]->size / REGISTER_SIZE - 1; j++)
        {
            mov_mem_reg(right_reg, registers.top(), offset);
            mov_reg_mem(registers.top(), left_reg, offset);
            offset -= REGISTER_SIZE;
        }
        mov_mem_reg(right_reg, registers.top());
        mov_reg_mem(registers.top(), left_reg);
        registers.push(left_reg);
        registers.push(right_reg);
    }
    else
    {
        if (node->right_exp->astnode_type == INTCONST_ASTNODE)
        {
            if (node->left_exp->astnode_type == IDENTIFIER_ASTNODE)
            {
                identifier_astnode *identifier_node = (identifier_astnode *)node->left_exp;
                if (isArray(identifier_node->exp_type) && symbTab->entries[identifier_node->name]->offset > 0)
                {
                    mov_mem_reg(EBP, registers.top(), symbTab->entries[identifier_node->name]->offset);
                    mov_con_mem(std::to_string(((intconst_astnode *)(node->right_exp))->value), registers.top());
                }
                else
                {
                    mov_con_mem(std::to_string(((intconst_astnode *)(node->right_exp))->value), EBP, symbTab->entries[identifier_node->name]->offset);
                }
            }
            else
            {
                gen(node->left_exp, 0);
                mov_con_mem(std::to_string(((intconst_astnode *)(node->right_exp))->value), registers.top());
            }
        }
        else
        {
            gen(node->right_exp, 1);
            right_reg = registers.top();
            registers.pop();
            if (node->left_exp->astnode_type == IDENTIFIER_ASTNODE)
            {
                identifier_astnode *identifier_node = (identifier_astnode *)node->left_exp;
                if (isArray(identifier_node->exp_type) && symbTab->entries[identifier_node->name]->offset > 0)
                {
                    mov_mem_reg(EBP, registers.top(), symbTab->entries[identifier_node->name]->offset);
                    mov_reg_mem(right_reg, registers.top());
                }
                else
                {
                    mov_reg_mem(right_reg, EBP, symbTab->entries[identifier_node->name]->offset);
                }
            }
            else
            {
                gen(node->left_exp, 0);
                mov_reg_mem(right_reg, registers.top());
            }
            registers.push(right_reg);
        }
    }
    return assignE_attr;
};

struct attributes gen_funcall(funcall_astnode *node, int value)
{
    struct attributes funcall_attr;
    int reg_active = NUM_REGISTERS - registers.size();
    int param_size = 0;
    std::string reg_top = registers.top();
    for (int i = 0; i < reg_active; i++)
    {
        save_reg(REGISTERS[reg_active - 1 - i]);
    }
    op_con_reg(MINUS_, std::to_string(size(gst->entries[node->func_identifier->name]->type)), ESP);
    for (uint i = 0; i < node->param_exps.size(); i++)
    {
        if (isArray(node->param_exps[i]->exp_type))
        {
            gen(node->param_exps[i], 0);
            push_reg(registers.top());
        }
        else if (isStruct(node->param_exps[i]->exp_type))
        {
            gen(node->param_exps[i], 0);
            std::string addr_reg = registers.top();
            registers.pop();
            int offset = gst->entries[node->param_exps[i]->exp_type]->size - REGISTER_SIZE;
            for (int j = 0; j < gst->entries[node->param_exps[i]->exp_type]->size / REGISTER_SIZE - 1; j++)
            {
                mov_mem_reg(addr_reg, registers.top(), offset);
                push_reg(registers.top());
                param_size++;
                offset -= REGISTER_SIZE;
            }
            registers.push(addr_reg);
            mov_mem_reg(registers.top(), registers.top());
            push_reg(registers.top());
        }
        else if (node->param_exps[i]->astnode_type == INTCONST_ASTNODE)
        {
            push_con(std::to_string(((intconst_astnode *)node->param_exps[i])->value));
        }
        else if (node->param_exps[i]->astnode_type == IDENTIFIER_ASTNODE)
        {
            push_mem(EBP, symbTab->entries[((identifier_astnode *)node->param_exps[i])->name]->offset);
        }
        else
        {
            gen(node->param_exps[i], 1);
            push_reg(registers.top());
        }
        param_size++;
    }
    call_fun(node->func_identifier->name);
    if (param_size)
        op_con_reg(PLUS_, std::to_string(REGISTER_SIZE * param_size), ESP);
    if (value)
    {
        mov_mem_reg(ESP, reg_top);
    }
    else
        mov_reg_reg(ESP, reg_top);
    op_con_reg(PLUS_, std::to_string(size(gst->entries[node->func_identifier->name]->type)), ESP);
    for (int i = 0; i < reg_active; i++)
    {
        restore_reg(REGISTERS[i]);
        registers.pop();
    }
    return funcall_attr;
};

struct attributes gen_intconst(intconst_astnode *node, int value)
{
    struct attributes intconst_attr;
    mov_con_reg(std::to_string(node->value), registers.top());
    return intconst_attr;
};

struct attributes gen_stringconst(stringconst_astnode *node, int value)
{
    struct attributes stringconst_attr;
    mov_con_reg(node->value, registers.top());
    return stringconst_attr;
};

struct attributes gen_seq(seq_astnode *node, int value)
{
    struct attributes seq_attr;
    for (auto child_node : node->statements)
    {
        if (seq_attr.next.size())
        {
            std::string next_label = gen_jmp_label();
            backpatch(seq_attr.next, next_label);
            instructions.push_back(next_label + ":");
        }
        struct attributes child_attr = gen(child_node, 0);
        seq_attr.next = child_attr.next;
    }
    if (value && seq_attr.next.size())
    {
        std::string next_label = gen_jmp_label();
        backpatch(seq_attr.next, next_label);
        instructions.push_back(next_label + ":");
    }
    return seq_attr;
};

struct attributes gen_assignS(assignS_astnode *node, int value)
{
    struct attributes assignS_attr;
    std::string left_reg, right_reg;

    if (isStruct(node->right_exp->exp_type))
    {
        gen(node->right_exp, 0);
        right_reg = registers.top();
        registers.pop();
        gen(node->left_exp, 0);
        left_reg = registers.top();
        registers.pop();
        int offset = gst->entries[node->right_exp->exp_type]->size - REGISTER_SIZE;
        for (int j = 0; j < gst->entries[node->right_exp->exp_type]->size / REGISTER_SIZE - 1; j++)
        {
            mov_mem_reg(right_reg, registers.top(), offset);
            mov_reg_mem(registers.top(), left_reg, offset);
            offset -= REGISTER_SIZE;
        }
        mov_mem_reg(right_reg, registers.top());
        mov_reg_mem(registers.top(), left_reg);
        registers.push(left_reg);
        registers.push(right_reg);
    }
    else
    {
        if (node->right_exp->astnode_type == INTCONST_ASTNODE)
        {
            if (node->left_exp->astnode_type == IDENTIFIER_ASTNODE)
            {
                identifier_astnode *identifier_node = (identifier_astnode *)node->left_exp;
                if (isArray(identifier_node->exp_type) && symbTab->entries[identifier_node->name]->offset > 0)
                {
                    mov_mem_reg(EBP, registers.top(), symbTab->entries[identifier_node->name]->offset);
                    mov_con_mem(std::to_string(((intconst_astnode *)(node->right_exp))->value), registers.top());
                }
                else
                {
                    mov_con_mem(std::to_string(((intconst_astnode *)(node->right_exp))->value), EBP, symbTab->entries[identifier_node->name]->offset);
                }
            }
            else
            {
                gen(node->left_exp, 0);
                mov_con_mem(std::to_string(((intconst_astnode *)(node->right_exp))->value), registers.top());
            }
        }
        else
        {
            gen(node->right_exp, 1);
            right_reg = registers.top();
            registers.pop();
            if (node->left_exp->astnode_type == IDENTIFIER_ASTNODE)
            {
                identifier_astnode *identifier_node = (identifier_astnode *)node->left_exp;
                if (isArray(identifier_node->exp_type) && symbTab->entries[identifier_node->name]->offset > 0)
                {
                    mov_mem_reg(EBP, registers.top(), symbTab->entries[identifier_node->name]->offset);
                    mov_reg_mem(right_reg, registers.top());
                }
                else
                {
                    mov_reg_mem(right_reg, EBP, symbTab->entries[identifier_node->name]->offset);
                }
            }
            else
            {
                gen(node->left_exp, 0);
                mov_reg_mem(right_reg, registers.top());
            }
            registers.push(right_reg);
        }
    }
    return assignS_attr;
};

struct attributes gen_return(return_astnode *node, int value)
{
    struct attributes return_attr;
    int return_offset = PARAM_OFFSET;
    for (uint i = 0; i < func_params[curr_func_name].size(); i++)
    {
        return_offset += isArray(func_params[curr_func_name][i]->type->type) ? ADDRESS_SIZE : func_params[curr_func_name][i]->type->size;
    }
    if (isStruct(node->return_exp->exp_type))
    {
        gen(node->return_exp, 0);
        std::string addr_reg = registers.top();
        registers.pop();
        int offset = gst->entries[node->return_exp->exp_type]->size - REGISTER_SIZE;
        for (int j = 0; j < gst->entries[node->return_exp->exp_type]->size / REGISTER_SIZE - 1; j++)
        {
            mov_mem_reg(addr_reg, registers.top(), offset);
            mov_reg_mem(registers.top(), EBP, return_offset + offset);
            offset -= REGISTER_SIZE;
        }
        registers.push(addr_reg);
        mov_mem_reg(registers.top(), registers.top());
        mov_reg_mem(registers.top(), EBP, return_offset);
    }
    else
    {
        gen(node->return_exp, 1);
        mov_reg_mem(registers.top(), EBP, return_offset);
    }
    returnlist.push_back(nextInstr());
    jmp_uncond();
    return return_attr;
};

struct attributes gen_if(if_astnode *node, int value)
{
    struct attributes if_attr;
    struct attributes cond_attr = gen_boolean(node->cond_exp, 1);
    if (cond_attr.truelist.size())
    {
        std::string then_label = gen_jmp_label();
        backpatch(cond_attr.truelist, then_label);
        instructions.push_back(then_label + ":");
    }
    struct attributes then_attr = gen(node->then_statement, 0);
    if_attr.next.push_back(nextInstr());
    jmp_uncond();
    if (cond_attr.falselist.size())
    {
        std::string else_label = gen_jmp_label();
        backpatch(cond_attr.falselist, else_label);
        instructions.push_back(else_label + ":");
    }
    struct attributes else_attr = gen(node->else_statement, 0);
    if_attr.next = merge(if_attr.next, then_attr.next);
    if_attr.next = merge(if_attr.next, else_attr.next);
    if (value && if_attr.next.size())
    {
        std::string next_label = gen_jmp_label();
        backpatch(if_attr.next, next_label);
        instructions.push_back(next_label + ":");
        if_attr.next = std::vector<int>();
    }
    return if_attr;
};

struct attributes gen_while(while_astnode *node, int value)
{
    struct attributes while_attr;
    std::string cond_label = gen_jmp_label();
    instructions.push_back(cond_label + ":");
    struct attributes cond_attr = gen_boolean(node->cond_exp, 1);
    struct attributes stmt_attr = gen(node->while_statement, 0);
    if (cond_attr.truelist.size())
    {
        std::string stmt_label = gen_jmp_label();
        backpatch(cond_attr.truelist, stmt_label);
        instructions.push_back(stmt_label + ":");
    }
    backpatch(stmt_attr.next, cond_label);
    while_attr.next = cond_attr.falselist;
    jmp_uncond(cond_label);
    if (value && while_attr.next.size())
    {
        std::string next_label = gen_jmp_label();
        backpatch(while_attr.next, next_label);
        instructions.push_back(next_label + ":");
        while_attr.next = std::vector<int>();
    }
    return while_attr;
};

struct attributes gen_for(for_astnode *node, int value)
{
    struct attributes for_attr;
    gen(node->init_exp, 1);
    std::string guard_label = gen_jmp_label();
    instructions.push_back(guard_label + ":");
    struct attributes guard_attr = gen_boolean(node->guard_exp, 1);
    if (guard_attr.truelist.size())
    {
        std::string stmt_label = gen_jmp_label();
        instructions.push_back(stmt_label + ":");
        backpatch(guard_attr.truelist, stmt_label);
    }
    struct attributes stmt_attr = gen(node->for_statement, 1);
    struct attributes step_attr = gen(node->step_exp, 0);
    backpatch(step_attr.next, guard_label);
    for_attr.next = guard_attr.falselist;
    jmp_uncond(guard_label);
    if (value && for_attr.next.size())
    {
        std::string next_label = gen_jmp_label();
        backpatch(for_attr.next, next_label);
        instructions.push_back(next_label + ":");
        for_attr.next = std::vector<int>();
    }
    return for_attr;
};

struct attributes gen_proccall(proccall_astnode *node, int value)
{
    struct attributes proccall_attr;
    if (node->proc_identifier->name == FUNC_PRINTF)
    {
        for (uint i = 0; i < node->param_exps.size() - 1; i++)
        {
            if (node->param_exps[node->param_exps.size() - 1 - i]->astnode_type == INTCONST_ASTNODE)
            {
                push_con(std::to_string(((intconst_astnode *)node->param_exps[node->param_exps.size() - 1 - i])->value));
            }
            else if (node->param_exps[node->param_exps.size() - 1 - i]->astnode_type == IDENTIFIER_ASTNODE)
            {
                push_mem(EBP, symbTab->entries[((identifier_astnode *)node->param_exps[node->param_exps.size() - 1 - i])->name]->offset);
            }
            else
            {
                gen(node->param_exps[node->param_exps.size() - 1 - i], 1);
                push_reg(registers.top());
            }
        }
        push_con(((stringconst_astnode *)node->param_exps[0])->value);
        call_fun(node->proc_identifier->name);
        if (node->param_exps.size())
            op_con_reg(PLUS_, std::to_string(REGISTER_SIZE * node->param_exps.size()), ESP);
    }
    else
    {
        int reg_active = NUM_REGISTERS - registers.size();
        int param_size = 0;
        std::string reg_top = registers.top();
        for (int i = 0; i < reg_active; i++)
        {
            save_reg(REGISTERS[reg_active - 1 - i]);
        }
        op_con_reg(MINUS_, std::to_string(size(gst->entries[node->proc_identifier->name]->type)), ESP);
        for (uint i = 0; i < node->param_exps.size(); i++)
        {
            if (isArray(node->param_exps[i]->exp_type))
            {
                gen(node->param_exps[i], 0);
                push_reg(registers.top());
            }
            else if (isStruct(node->param_exps[i]->exp_type))
            {
                gen(node->param_exps[i], 0);
                std::string addr_reg = registers.top();
                registers.pop();
                int offset = gst->entries[node->param_exps[i]->exp_type]->size - REGISTER_SIZE;
                for (int j = 0; j < gst->entries[node->param_exps[i]->exp_type]->size / REGISTER_SIZE - 1; j++)
                {
                    mov_mem_reg(addr_reg, registers.top(), offset);
                    push_reg(registers.top());
                    param_size++;
                    offset -= REGISTER_SIZE;
                }
                registers.push(addr_reg);
                mov_mem_reg(registers.top(), registers.top());
                push_reg(registers.top());
            }
            else if (node->param_exps[i]->astnode_type == INTCONST_ASTNODE)
            {
                push_con(std::to_string(((intconst_astnode *)node->param_exps[i])->value));
            }
            else if (node->param_exps[i]->astnode_type == IDENTIFIER_ASTNODE)
            {
                push_mem(EBP, symbTab->entries[((identifier_astnode *)node->param_exps[i])->name]->offset);
            }
            else
            {
                gen(node->param_exps[i], 1);
                push_reg(registers.top());
            }
            param_size++;
        }
        call_fun(node->proc_identifier->name);
        if (param_size)
            op_con_reg(PLUS_, std::to_string(REGISTER_SIZE * param_size), ESP);
        if (value)
        {
            mov_mem_reg(ESP, reg_top);
        }
        else
            mov_reg_reg(ESP, reg_top);
        op_con_reg(PLUS_, std::to_string(size(gst->entries[node->proc_identifier->name]->type)), ESP);
        for (int i = 0; i < reg_active; i++)
        {
            restore_reg(REGISTERS[i]);
            registers.pop();
        }
    }
    return proccall_attr;
};

struct attributes gen(abstract_astnode *node, int value)
{
    switch (node->astnode_type)
    {
    case IDENTIFIER_ASTNODE:
        return gen_identifier((identifier_astnode *)node, value);
    case ARRAYREF_ASTNODE:
        return gen_arrayref((arrayref_astnode *)node, value);
    case MEMBER_ASTNODE:
        return gen_member((member_astnode *)node, value);
    case ARROW_ASTNODE:
        return gen_arrow((arrow_astnode *)node, value);
    case OP_BINARY_ASTNODE:
        return gen_op_binary((op_binary_astnode *)node, value);
    case OP_UNARY_ASTNODE:
        return gen_op_unary((op_unary_astnode *)node, value);
    case ASSIGNE_ASTNODE:
        return gen_assignE((assignE_astnode *)node, value);
    case FUNCALL_ASTNODE:
        return gen_funcall((funcall_astnode *)node, value);
    case INTCONST_ASTNODE:
        return gen_intconst((intconst_astnode *)node, value);
    case STRINGCONST_ASTNODE:
        return gen_stringconst((stringconst_astnode *)node, value);
    case SEQ_ASTNODE:
        return gen_seq((seq_astnode *)node, value);
    case ASSIGNS_ASTNODE:
        return gen_assignS((assignS_astnode *)node, value);
    case RETURN_ASTNODE:
        return gen_return((return_astnode *)node, value);
    case IF_ASTNODE:
        return gen_if((if_astnode *)node, value);
    case WHILE_ASTNODE:
        return gen_while((while_astnode *)node, value);
    case FOR_ASTNODE:
        return gen_for((for_astnode *)node, value);
    case PROCCALL_ASTNODE:
        return gen_proccall((proccall_astnode *)node, value);
    default:
        struct attributes attr;
        return attr;
    }
}

void gen_func(std::string func_name)
{
    for (int i = 0; i < NUM_REGISTERS; i++)
    {
        registers.push(REGISTERS[NUM_REGISTERS - 1 - i]);
    }
    gen_node_rodata(ast[func_name]);
    gen_func_prolog(func_name);

    symbTab = gst->entries[func_name]->symbtab;
    instructions = std::vector<std::string>();
    returnlist = std::vector<int>();
    stack_size = gst->entries[func_name]->size;
    curr_func_name = func_name;

    apply_constant_folding(ast[func_name]);

    push_reg(EBP);
    mov_reg_reg(ESP, EBP);
    if (stack_size)
        op_con_reg(MINUS_, std::to_string(stack_size), ESP);

    gen(ast[func_name], 1);

    if (instructions[instructions.size() - 1] == "\tjmp\t")
    {
        instructions.pop_back();
        returnlist.pop_back();
    }
    if (returnlist.size())
    {
        std::string next_label = gen_jmp_label();
        backpatch(returnlist, next_label);
        instructions.push_back(next_label + ":");
    }

    for (auto inst : instructions)
    {
        std::cout << inst << "\n";
    }

    gen_func_epilog(func_name);
}