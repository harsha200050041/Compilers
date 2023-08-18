#include "type.hh"

type_struct::type_struct(std::string type, int size)
{
     this->type = type;
     this->size = size;
}

decl_struct::decl_struct(std::string name, struct type_struct *type)
{
     this->name = name;
     this->type = type;
}

bool isArray(std::string type)
{
     return type.find("(") == std::string::npos && type.find("[") != std::string::npos;
}

bool isPointer(std::string type)
{
     return type.find("*") != std::string::npos && !isArray(type);
}

bool isScalar(std::string type)
{
     if (type == INT_TYPE || type == FLOAT_TYPE || type == STRING_TYPE || isArray(type) || isPointer(type))
          return true;

     return false;
};

std::string arrayToPointer(std::string type)
{
     if (!isArray(type))
          return type;
     type.erase(type.begin() + type.find("[") + 1, type.begin() + type.find("]"));
     type.insert(type.find("[") + 1, "*");
     type.replace(type.find("["), 1, "(");
     type.replace(type.find("]"), 1, ")");
     return type;
}

std::string pointerToPointer(std::string type)
{
     if (!isPointer(type))
          return type;
     if (type.find("(") != std::string::npos)
          return type;
     type.pop_back();
     return type + "(*)";
}

std::string pointer(std::string type)
{
     if (type.find("(") != std::string::npos)
     {
          type.insert(type.find("(") + 1, "*");
     }
     else if (type.find("[") != std::string::npos)
     {
          type.insert(type.find("["), "(*)");
     }
     else
     {
          type += "(*)";
     }
     return type;
}

std::string deref(std::string type)
{
     if (isArray(type))
     {
          type.erase(type.begin() + type.find("["), type.begin() + type.find("]") + 1);
     }
     else if (isPointer(type))
     {
          if (type.find("(*)") != std::string::npos)
          {
               type.erase(type.begin() + type.find("("), type.begin() + type.find(")") + 1);
          }
          else
          {
               int last_asterisk = -1;
               for (uint i = 0; i < type.size(); i++)
               {
                    if (type[i] == '*')
                         last_asterisk = i;
               }
               type.erase(type.begin() + last_asterisk);
          }
     }
     return type;
}

bool compatibleAssignment(std::string left_type, std::string right_type)
{
     if (left_type == right_type)
          return true;

     if (left_type == VOID_POINTER_TYPE && (isArray(right_type) || isPointer(right_type)))
          return true;

     if (isPointer(left_type) && right_type == VOID_POINTER_TYPE)
          return true;

     if (isPointer(left_type) && isArray(right_type) && pointerToPointer(left_type) == arrayToPointer(right_type))
          return true;

     if (isPointer(left_type) && isPointer(right_type) && pointerToPointer(left_type) == pointerToPointer(right_type))
          return true;

     return false;
}

bool compatible(std::string left_type, std::string right_type)
{
     if (left_type == right_type && isScalar(left_type) && isScalar(right_type))
          return true;

     if (isPointer(left_type) && isArray(right_type) && pointerToPointer(left_type) == arrayToPointer(right_type))
          return true;

     if (isArray(left_type) && isPointer(right_type) && arrayToPointer(left_type) == pointerToPointer(right_type))
          return true;

     if (isArray(left_type) && isArray(right_type) && arrayToPointer(left_type) == arrayToPointer(right_type))
          return true;

     if (isPointer(left_type) && isPointer(right_type) && pointerToPointer(left_type) == pointerToPointer(right_type))
          return true;

     return false;
}
