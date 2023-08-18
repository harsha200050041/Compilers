#ifndef SYMBTAB_H
#define SYMBTAB_H
#include <bits/stdc++.h>

const std::map<std::string, std::string> PREDEFINED{{"printf", "void"}, {"scanf", "void"}, {"mod", "int"}};
const std::string GLOBAL_SCOPE = "global";
const std::string LOCAL_SCOPE = "local";
const std::string PARAM_SCOPE = "param";

class SymbTab;

struct SymbTabEntry
{
    std::string name;
    std::string varfun;
    std::string scope;
    int size;
    int offset;
    std::string type;
    SymbTab *symbtab;
    SymbTabEntry(std::string name, std::string varfun, std::string scope, int size, int offset, std::string type, SymbTab *symbtab);
};

class SymbTab
{
public:
    std::map<std::string, SymbTabEntry *> entries;

    void print(int blanks);
};

bool present(SymbTab* st, std::string name);

#endif