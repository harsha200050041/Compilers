#ifndef TYPE_H
#define TYPE_H
#include <bits/stdc++.h>

const int INT_SIZE = 4;
const int FLOAT_SIZE = 4;
const int ADDRESS_SIZE = 4;

const int LOCAL_OFFSET = 0;
const int STRUCT_OFFSET = 0;
const int PARAM_OFFSET = 12;

const std::string INT_TYPE = "int";
const std::string FLOAT_TYPE = "float";
const std::string STRING_TYPE = "string";
const std::string VOID_TYPE = "void";
const std::string VOID_POINTER_TYPE = VOID_TYPE + "*";

const std::string STRUCT_TYPE = "struct";
const std::string FUN_TYPE = "fun";
const std::string VAR_TYPE = "var";

struct type_struct
{
    std::string type;
    int size;
    type_struct(std::string type, int size);
};

struct decl_struct
{
    std::string name;
    struct type_struct *type;
    decl_struct(std::string name, struct type_struct *type);
};

bool isArray(std::string type);
bool isPointer(std::string type);
bool isScalar(std::string type);

std::string arrayToPointer(std::string type);
std::string pointerToPointer(std::string type);
std::string pointer(std::string type);
std::string deref(std::string type);

bool compatibleAssignment(std::string left_type, std::string right_type);
bool compatible(std::string left_type, std::string right_type);

#endif