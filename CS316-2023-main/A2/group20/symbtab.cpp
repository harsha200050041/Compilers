#include "symbtab.hh"

extern int BLANK_SIZE;
extern std::string BLANK_STRING;

SymbTabEntry::SymbTabEntry(std::string name, std::string varfun, std::string scope, int size, int offset, std::string type, SymbTab *symbtab)
{
    this->name = name;
    this->varfun = varfun;
    this->scope = scope;
    this->size = size;
    this->offset = offset;
    this->type = type;
    this->symbtab = symbtab;
};

void SymbTab::print(int blanks)
{
    std::string blank_string = std::string(blanks, ' ');
    if (this->entries.size())
    {
        std::cout << "[\n";
        for (auto it = this->entries.begin(); it != this->entries.end(); it++)
        {
            if (it != this->entries.begin())
            {
                std::cout << ",\n";
            }
            std::cout << blank_string << BLANK_STRING << "[\n"
                      << blank_string << BLANK_STRING << BLANK_STRING << "\"" << it->second->name << "\",\n"
                      << blank_string << BLANK_STRING << BLANK_STRING << "\"" << it->second->varfun << "\",\n"
                      << blank_string << BLANK_STRING << BLANK_STRING << "\"" << it->second->scope << "\",\n"
                      << blank_string << BLANK_STRING << BLANK_STRING << it->second->size << ",\n"
                      << blank_string << BLANK_STRING << BLANK_STRING << (it->second->varfun == "struct" ? "\"-\"" : std::to_string(it->second->offset)) << ",\n"
                      << blank_string << BLANK_STRING << BLANK_STRING << "\"" << (it->second->varfun == "struct" ? "-" : it->second->type) << "\"\n"
                      << blank_string << BLANK_STRING << "]";
        }
        std::cout << "\n"
                  << blank_string << "]";
    }
    else
    {
        std::cout << "[]\n";
    }
}

bool present(SymbTab *st, std::string name)
{
    return st->entries.find(name) != st->entries.end();
}