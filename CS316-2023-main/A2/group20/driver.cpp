#include <cstring>
#include <cstddef>
#include <istream>
#include <iostream>
#include <fstream>

#include "scanner.hh"
#include "parser.tab.hh"
using namespace std;

int BLANK_SIZE = 4;
std::string BLANK_STRING = std::string(BLANK_SIZE, ' ');

extern map<string, abstract_astnode *> ast;
extern SymbTab *gst;

int main(int argc, char **argv)
{
     fstream in_file, out_file;
     in_file.open(argv[1], ios::in);

     IPL::Scanner scanner(in_file);
     IPL::Parser parser(scanner);

#ifdef YYDEBUG
     parser.set_debug_level(1);
#endif
     parser.parse();

     SymbTab *gstStructs = new SymbTab(), *gstFunctions = new SymbTab();
     for (auto e : gst->entries)
     {
          if (e.second->varfun == "struct")
          {
               gstStructs->entries.insert(e);
          }
          if (e.second->varfun == "fun")
          {
               gstFunctions->entries.insert(e);
          }
     }

     // start the JSON printing
     cout << "{\n"
          << BLANK_STRING << "\"globalST\": ";
     gst->print(BLANK_SIZE);
     cout << ",\n";
     if (gstStructs->entries.size())
     {
          cout << BLANK_STRING << "\"structs\": [\n";
          for (auto it = gstStructs->entries.begin(); it != gstStructs->entries.end(); it++)
          {
               if (it != gstStructs->entries.begin())
               {
                    cout << ",\n";
               }
               cout << BLANK_STRING << BLANK_STRING << "{\n"
                    << BLANK_STRING << BLANK_STRING << BLANK_STRING << "\"name\": \"" << it->first << "\",\n"
                    << BLANK_STRING << BLANK_STRING << BLANK_STRING << "\"localST\": ";
               it->second->symbtab->print(3 * BLANK_SIZE);
               cout << "\n"
                    << BLANK_STRING << BLANK_STRING << "}";
          }
          cout << "\n"
               << BLANK_STRING << "],\n";
     }
     else
     {
          cout << BLANK_STRING << "\"structs\": [],\n";
     }
     if (gstFunctions->entries.size())
     {
          cout << BLANK_STRING << "\"functions\": [\n";
          for (auto it = gstFunctions->entries.begin(); it != gstFunctions->entries.end(); it++)
          {
               if (it != gstFunctions->entries.begin())
               {
                    cout << ",\n";
               }
               cout << BLANK_STRING << BLANK_STRING << "{\n"
                    << BLANK_STRING << BLANK_STRING << BLANK_STRING << "\"name\": \"" << it->first << "\",\n"
                    << BLANK_STRING << BLANK_STRING << BLANK_STRING << "\"localST\": ";
               it->second->symbtab->print(3 * BLANK_SIZE);
               cout << ",\n"
                    << BLANK_STRING << BLANK_STRING << BLANK_STRING << "\"ast\": ";
               ast[it->first]->print(3 * BLANK_SIZE);
               cout << "\n"
                    << BLANK_STRING << BLANK_STRING << "}";
          }
          cout << "\n"
               << BLANK_STRING << "]\n";
     }
     else
     {
          cout << BLANK_STRING << "\"functions\": []\n";
     }
     cout << "}";

     fclose(stdout);

     exit(0);
}