#ifndef ASM_H
#define ASM_H
#include <bits/stdc++.h>

#include "ast.hh"
#include "symbtab.hh"
#include "type.hh"

struct attributes
{
    std::vector<int> truelist;
    std::vector<int> falselist;
    std::vector<int> next;
};

struct attributes gen(abstract_astnode *node, int value);
struct attributes gen_boolean(abstract_astnode *node, int fall);
void gen_func(std::string func_name);

#endif
