%skeleton "lalr1.cc"
%require  "3.0.1"

%defines 
%define api.namespace {IPL}
%define api.parser.class {Parser}

%define parse.trace

%code requires{
     #include "type.hh"
     #include "symbtab.hh"
     #include "ast.hh"
     #include "location.hh"

     namespace IPL {
          class Scanner;
     }
}

%printer { std::cerr << $$; } STRUCT
%printer { std::cerr << $$; } VOID
%printer { std::cerr << $$; } INT
%printer { std::cerr << $$; } FLOAT
%printer { std::cerr << $$; } RETURN
%printer { std::cerr << $$; } IF
%printer { std::cerr << $$; } ELSE
%printer { std::cerr << $$; } WHILE
%printer { std::cerr << $$; } FOR
%printer { std::cerr << $$; } IDENTIFIER
%printer { std::cerr << $$; } INT_CONSTANT
%printer { std::cerr << $$; } FLOAT_CONSTANT
%printer { std::cerr << $$; } STRING_LITERAL
%printer { std::cerr << $$; } OR_OP
%printer { std::cerr << $$; } AND_OP
%printer { std::cerr << $$; } EQ_OP
%printer { std::cerr << $$; } NE_OP
%printer { std::cerr << $$; } LE_OP
%printer { std::cerr << $$; } GE_OP
%printer { std::cerr << $$; } INC_OP
%printer { std::cerr << $$; } PTR_OP
%printer { std::cerr << $$; } OTHERS

%parse-param { Scanner  &scanner  }
%locations
%code{
     #include <iostream>
     #include <cstdlib>
     #include <fstream>
     #include <string>

     #include "scanner.hh"
     
     std::map<std::string, abstract_astnode*> ast;
     SymbTab *gst = new SymbTab(), *lst = NULL;
     int offset = 0;
     struct type_struct *type = NULL, *base_type = NULL;
     std::pair<std::string, struct decl_struct*> entity_type = {"", NULL};
     std::map<std::string, std::vector<decl_struct*>> func_params;

     op_unary_astnode *int_cast(exp_astnode *exp)
     {
          return new op_unary_astnode(INT_TYPE, TO_INT, exp);
     };
     op_unary_astnode *float_cast(exp_astnode *exp)
     {
          return new op_unary_astnode(FLOAT_TYPE, TO_FLOAT, exp);
     };

#undef yylex
#define yylex IPL::Parser::scanner.yylex
}

%define api.value.type variant
%define parse.assert

%start translation_unit

%token <std::string> STRUCT
%token <std::string> VOID
%token <std::string> INT
%token <std::string> FLOAT
%token <std::string> RETURN
%token <std::string> IF
%token <std::string> ELSE
%token <std::string> WHILE
%token <std::string> FOR
%token <std::string> IDENTIFIER
%token <int> INT_CONSTANT
%token <float> FLOAT_CONSTANT
%token <std::string> STRING_LITERAL
%token <std::string> OR_OP
%token <std::string> AND_OP
%token <std::string> EQ_OP
%token <std::string> NE_OP
%token <std::string> LE_OP
%token <std::string> GE_OP
%token <std::string> INC_OP
%token <std::string> PTR_OP
%token <std::string> OTHERS
%token '{' '}' '(' ')' '[' ']' ';' ',' '+' '-' '*' '/' '!' '&' '=' '<' '>' '.'

%nterm <exp_astnode*> expression unary_expression logical_and_expression equality_expression relational_expression additive_expression multiplicative_expression primary_expression postfix_expression
%nterm <assignE_astnode*> assignment_expression
%nterm <std::vector<exp_astnode*>> expression_list

%nterm <statement_astnode*> statement iteration_statement selection_statement 
%nterm <seq_astnode*> compound_statement
%nterm <assignS_astnode*> assignment_statement
%nterm <proccall_astnode*> procedure_call
%nterm <std::vector<statement_astnode*>> statement_list

%nterm <decl_struct*> fun_declarator declarator declarator_arr parameter_declaration
%nterm <std::vector<decl_struct*>> parameter_list

%nterm <unary_op> unary_operator

%%
translation_unit: 
     struct_specifier
     {
     }
     | function_definition
     {
     }
     | translation_unit struct_specifier
     {
     }
     | translation_unit function_definition
     {
     }
     ;

struct_specifier: 
     STRUCT IDENTIFIER 
     { 
          lst = new SymbTab(); 
          offset = STRUCT_OFFSET; 
          if (!present(gst, $1 + " " + $2))
          {
               entity_type = {STRUCT_TYPE, new decl_struct("", new type_struct($1 + " " + $2, 0))};
          }
          else
          {
               error(@$, "\"" + $1 + " " + $2 + "\" has a previous definition");
          }
     }  '{' declaration_list '}' ';'
     {
          gst->entries.insert({$1 + " " + $2, new SymbTabEntry($1 + " " + $2, STRUCT_TYPE, GLOBAL_SCOPE, offset, 0, "", lst)});
     }
     ;

function_definition: 
     type_specifier { lst = new SymbTab(); offset = PARAM_OFFSET; entity_type = {FUN_TYPE, new decl_struct("", base_type)}; } fun_declarator { offset = LOCAL_OFFSET; entity_type.second->name = $3->name; } compound_statement
     {
          ast.insert({$3->name, $5});
          gst->entries.insert({$3->name, new SymbTabEntry($3->name, FUN_TYPE, GLOBAL_SCOPE, 0, 0, $3->type->type, lst)});
     }
     ;

type_specifier: 
     VOID
     {
          base_type = new type_struct(VOID_TYPE, 0);
     }
     | INT
     {
          base_type = new type_struct(INT_TYPE, INT_SIZE);
     }
     | FLOAT
     {
          base_type = new type_struct(FLOAT_TYPE, FLOAT_SIZE);
     }
     | STRUCT IDENTIFIER
     {
          if (present(gst, $1 + " " + $2))
          {
               base_type = new type_struct($1 + " " + $2, gst->entries[$1 + " " + $2]->size);
          }
          else if (entity_type.first == STRUCT_TYPE && entity_type.second->type->type == $1 + " " + $2)
          {
               base_type = new type_struct($1 + " " + $2, 0);
          }
          else
          {
               error(@$, "\"" + $1 + " " + $2 + "\" is not defined");
          }
     }
     ;

fun_declarator: 
     IDENTIFIER '(' parameter_list ')'
     {
          func_params[$1] = $3;
          if (!present(gst, $1))
          {
               $$ = new decl_struct($1, entity_type.second->type);
               for (uint i = 0; i < $3.size(); i++)
               {
                    if (!present(lst, $3[i]->name))
                    {
                         offset -= $3[i]->type->size;
                         lst->entries.insert({$3[i]->name, new SymbTabEntry($3[i]->name, VAR_TYPE, PARAM_SCOPE, $3[i]->type->size, offset, $3[i]->type->type, NULL)});
                    }
                    else
                    {
                         error(@$, "\"" + $3[i]->name + "\" has a previous declaration");
                    }
               }
          }
          else
          {
               error(@$, "The function \"" + $1 + "\" has a previous definition");
          }
     }
     | IDENTIFIER '(' ')'
     {
          func_params[$1];
          if (!present(gst, $1))
          {
	          $$ = new decl_struct($1, entity_type.second->type
               );
          }
          else
          {
               error(@$, "'" + $1 + "' has a previous definition");
          }
     }
     ;

parameter_list: 
     parameter_declaration 
     {
          $$.push_back($1);
     }
     | parameter_list ',' parameter_declaration
     {
          $$ = $1;
          $$.push_back($3);
     }
     ;

parameter_declaration: 
     type_specifier { type = new type_struct(base_type->type, base_type->size); } declarator
     {
          offset += $3->type->size;
          $$ = $3;
     }
     ;

declarator_arr: 
     IDENTIFIER
     {
          if (type->type == VOID_TYPE)
          {
               error(@$, "Cannot declare variable of type \"void\"");
          }
          else if (entity_type.first == STRUCT_TYPE && type->size == 0)
          {
               error(@$, "\"" + type->type + "\" is not defined");
          }
          else if (!present(lst, $1))
          {
               $$ = new decl_struct($1, type);
          }
          else
          {
               error(@$, "\"" + $1 + "\" has a previous declaration");
          }
     }
     | declarator_arr '[' INT_CONSTANT ']'
     {
          $$ = $1;
          $$->type->type = $$->type->type + "[" + std::to_string($3) + "]";
          $$->type->size = $$->type->size * $3;
     }
     ;

declarator: 
     declarator_arr
     {
          $$ = $1;
     }
     | '*' { type->type = type->type + "*"; type->size = ADDRESS_SIZE; } declarator
     {
          $$ = $3;
     }
     ;

compound_statement: 
     '{' '}'
     {
          $$ = new seq_astnode(std::vector<statement_astnode*> ());
     }
     | '{' statement_list '}'
     {
          $$ = new seq_astnode($2);
     }
     | '{' declaration_list '}'
     {
          $$ = new seq_astnode(std::vector<statement_astnode*> ());
     }
     | '{' declaration_list statement_list '}'
     {	               
          $$ = new seq_astnode($3);
     }
     ;

statement_list: 
     statement
     {
	     $$.push_back($1);
     }
     | statement_list statement
     {
	     $$ = $1;
          $$.push_back($2);
     }
     ;

statement: 
     ';'
     {
	     $$ = new empty_astnode();
     }
     | '{' statement_list '}'
     {
	     $$ = new seq_astnode($2);
     }
     | selection_statement
     {
	     $$ = $1;
     }
     | iteration_statement
     {
	     $$ = $1;
     }
     | assignment_statement
     {
	     $$ = $1;
     }
     | procedure_call
     {
	     $$ = $1;
     }
     | RETURN expression ';'
     {
          if ($2->exp_type == entity_type.second->type->type)
          {
               $$ = new return_astnode($2);
          }
          else if ($2->exp_type == INT_TYPE && entity_type.second->type->type == FLOAT_TYPE)
          {
               $$ = new return_astnode(float_cast($2));
          }
          else if ($2->exp_type == FLOAT_TYPE && entity_type.second->type->type == INT_TYPE)
          {  
               $$ = new return_astnode(int_cast($2));
          }
          else
          {
               error(@$, "Incompatible type \"" + $2->exp_type + "\" returned, expected \"" + entity_type.second->type->type + "\"");
          }
     }
     ;

assignment_expression: 
     unary_expression '=' expression
     {
          if ($1->l_value == 0)
          {
               error(@$, "Left operand of assignment should have an lvalue");
          }
          else if (isArray($1->exp_type))
          {
               error(@$, "Invalid assignment to expression with array type");
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
	          $$ = new assignE_astnode($1->exp_type, $1, int_cast($3));
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
	          $$ = new assignE_astnode($1->exp_type, $1, float_cast($3));
          }
          else if (compatibleAssignment($1->exp_type, $3->exp_type))
          {
	          $$ = new assignE_astnode($1->exp_type, $1, $3);
          }
          else if (isPointer($1->exp_type) && $3->astnode_type == INTCONST_ASTNODE && ((intconst_astnode *)$3)->value == 0)
          {
	          $$ = new assignE_astnode($1->exp_type, $1, $3);
          }
          else
          {
               error(@$, "Incompatible assignment when assigning to type \"" + $1->exp_type + "\" from type \"" + $3->exp_type + "\"");
          }
     }
     ;

assignment_statement: 
     assignment_expression ';'
     {
	     $$ = new assignS_astnode($1);
     }
     ;

procedure_call: 
     IDENTIFIER '(' ')' ';'
     {
          if (present(gst, $1) || (entity_type.first == FUN_TYPE && entity_type.second->name == $1))
          {
               if (func_params[$1].size() == 0) 
               {
                    if (present(gst, $1))
                    {
                         $$ = new proccall_astnode(new identifier_astnode(gst->entries[$1]->type, $1), std::vector<exp_astnode*> ());
                    }
                    else
                    {
                         $$ = new proccall_astnode(new identifier_astnode(entity_type.second->type->type, $1), std::vector<exp_astnode*> ());
                    }
               }
               else
               {
                    error(@$, "Procedure \"" + $1 + "\" called with too few arguments");
               }
          }
          else if (PREDEFINED.find($1) != PREDEFINED.end())
          {
               $$ = new proccall_astnode(new identifier_astnode(PREDEFINED.at($1), $1), std::vector<exp_astnode*> ());
          }
          else
          {
               error(@$, "Procedure \"" + $1 + "\" is not declared");
          }
     }
     | IDENTIFIER '(' expression_list ')' ';'
     {
          if (present(gst, $1) || (entity_type.first == FUN_TYPE && entity_type.second->name == $1))
          {
               if (func_params[$1].size() == $3.size())
               {
                    for (uint i = 0; i < func_params[$1].size(); i++){
                         if (func_params[$1][i]->type->type == INT_TYPE && $3[i]->exp_type == FLOAT_TYPE)
                         {
                              $3[i] = int_cast($3[i]);
                         }
                         else if (func_params[$1][i]->type->type == FLOAT_TYPE && $3[i]->exp_type == INT_TYPE)
                         {
                              $3[i] = float_cast($3[i]);
                         }
                         else if (func_params[$1][i]->type->type == $3[i]->exp_type)
                         {
                              continue;
                         }
                         else if (func_params[$1][i]->type->type == VOID_POINTER_TYPE && (isArray($3[i]->exp_type) || isPointer($3[i]->exp_type)))
                         {
                              continue;
                         }
                         else if ((isArray(func_params[$1][i]->type->type) || isPointer(func_params[$1][i]->type->type)) && $3[i]->exp_type == VOID_POINTER_TYPE)
                         {
                              continue;
                         }      
                         else if (compatible(func_params[$1][i]->type->type, $3[i]->exp_type))
                         {
                              continue;
                         }
                         else if (isPointer(func_params[$1][i]->type->type) && $3[i]->astnode_type == INTCONST_ASTNODE && ((intconst_astnode*)$3[i])->value == 0)
                         {
                              continue;
                         }
                         else
                         {
                              error(@$, "Expected \"" + func_params[$1][i]->type->type + "\" but argument is of type \"" + $3[i]->exp_type + "\"");
                         }
                    }
                    if (present(gst, $1))
                    {
                         $$ = new proccall_astnode(new identifier_astnode(gst->entries[$1]->type, $1), $3);
                    }
                    else
                    {
                         $$ = new proccall_astnode(new identifier_astnode(entity_type.second->type->type, $1), $3);
                    }
               }
               else if (func_params[$1].size() > $3.size()) 
               {
                    error(@$, "Procedure \"" + $1 + "\" called with too few arguments");
               }
               else if (func_params[$1].size() < $3.size())
               {
                    error(@$, "Procedure \"" + $1 + "\" called with too many arguments");
               }
               else
               {
                    error(@$, "Not possible");
               }
          }
          else if (PREDEFINED.find($1) != PREDEFINED.end())
          {
	          $$ = new proccall_astnode(new identifier_astnode(PREDEFINED.at($1), $1), $3);
          }
          else
          {
               error(@$, "Procedure \"" + $1 + "\" is not declared");
          }
     }
     ;

expression: 
     logical_and_expression
     {
	     $$ = $1;
     }
     | expression OR_OP logical_and_expression
     {
          if (isScalar($1->exp_type) && isScalar($3->exp_type))
          {
     	     $$ = new op_binary_astnode(INT_TYPE, OR_OP, _NA, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary ||, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     ;

logical_and_expression: 
     equality_expression
     {
	     $$ = $1;
     }
     | logical_and_expression AND_OP equality_expression
     {
          if (isScalar($1->exp_type) && isScalar($3->exp_type))
          {
     	     $$ = new op_binary_astnode(INT_TYPE, AND_OP, _NA, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary &&, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     ;

equality_expression: 
     relational_expression
     {
	     $$ = $1;
     }
     | equality_expression EQ_OP relational_expression
     {
          if ($1->exp_type == FLOAT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, EQ_OP_, _FLOAT, $1, $3);
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, EQ_OP_, _FLOAT, float_cast($1), $3);
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, EQ_OP_, _FLOAT, $1, float_cast($3));
          }
          else if ($1->exp_type == VOID_POINTER_TYPE && (isArray($3->exp_type) || isPointer($3->exp_type)))
          {
               $$ = new op_binary_astnode(INT_TYPE, EQ_OP_, _INT, $1, $3);
          }
          else if ((isArray($1->exp_type) || isPointer($1->exp_type)) && $3->exp_type == VOID_POINTER_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, EQ_OP_, _INT, $1, $3);
          }      
          else if (compatible($1->exp_type, $3->exp_type))
          {
               $$ = new op_binary_astnode(INT_TYPE, EQ_OP_, _INT, $1, $3);
          }
          else if ((isPointer($1->exp_type) || isArray($1->exp_type)) && $3->astnode_type == INTCONST_ASTNODE && ((intconst_astnode*)$3)->value == 0)
          {
               $$ = new op_binary_astnode(INT_TYPE, EQ_OP_, _INT, $1, $3);
          }
          else if ((isPointer($3->exp_type) || isArray($3->exp_type)) && $1->astnode_type == INTCONST_ASTNODE && ((intconst_astnode*)$1)->value == 0)
          {
               $$ = new op_binary_astnode(INT_TYPE, EQ_OP_, _INT, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary ==, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     | equality_expression NE_OP relational_expression
     {
          
          if ($1->exp_type == FLOAT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, NE_OP_, _FLOAT, $1, $3);
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, NE_OP_, _FLOAT, float_cast($1), $3);
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, NE_OP_, _FLOAT, $1, float_cast($3));
          }
          else if ($1->exp_type == VOID_POINTER_TYPE && (isArray($3->exp_type) || isPointer($3->exp_type)))
          {
               $$ = new op_binary_astnode(INT_TYPE, NE_OP_, _INT, $1, $3);
          }
          else if ((isArray($1->exp_type) || isPointer($1->exp_type)) && $3->exp_type == VOID_POINTER_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, NE_OP_, _INT, $1, $3);
          }      
          else if (compatible($1->exp_type, $3->exp_type))
          {
               $$ = new op_binary_astnode(INT_TYPE, NE_OP_, _INT, $1, $3);
          }
          else if ((isPointer($1->exp_type) || isArray($1->exp_type)) && $3->astnode_type == INTCONST_ASTNODE && ((intconst_astnode*)$3)->value == 0)
          {
               $$ = new op_binary_astnode(INT_TYPE, NE_OP_, _INT, $1, $3);
          }
          else if ((isPointer($3->exp_type) || isArray($3->exp_type)) && $1->astnode_type == INTCONST_ASTNODE && ((intconst_astnode*)$1)->value == 0)
          {
               $$ = new op_binary_astnode(INT_TYPE, NE_OP_, _INT, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary !=, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     ;

relational_expression: 
     additive_expression
     {
	     $$ = $1;
     }
     | relational_expression '<' additive_expression
     {
          
          if ($1->exp_type == FLOAT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, LT_OP_, _FLOAT, $1, $3);
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, LT_OP_, _FLOAT, float_cast($1), $3);
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, LT_OP_, _FLOAT, $1, float_cast($3));
          }
          else if (compatible($1->exp_type, $3->exp_type))
          {
               $$ = new op_binary_astnode(INT_TYPE, LT_OP_, _INT, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary <, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     | relational_expression '>' additive_expression
     {
          
          if ($1->exp_type == FLOAT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, GT_OP_, _FLOAT, $1, $3);
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, GT_OP_, _FLOAT, float_cast($1), $3);
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, GT_OP_, _FLOAT, $1, float_cast($3));
          }
          else if (compatible($1->exp_type, $3->exp_type))
          {
               $$ = new op_binary_astnode(INT_TYPE, GT_OP_, _INT, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary >, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     | relational_expression LE_OP additive_expression
     {
          
          if ($1->exp_type == FLOAT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, LE_OP_, _FLOAT, $1, $3);
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, LE_OP_, _FLOAT, float_cast($1), $3);
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, LE_OP_, _FLOAT, $1, float_cast($3));
          }
          else if (compatible($1->exp_type, $3->exp_type))
          {
               $$ = new op_binary_astnode(INT_TYPE, LE_OP_, _INT, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary <=, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     | relational_expression GE_OP additive_expression
     {
          
          if ($1->exp_type == FLOAT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, GE_OP_, _FLOAT, $1, $3);
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, GE_OP_, _FLOAT, float_cast($1), $3);
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, GE_OP_, _FLOAT, $1, float_cast($3));
          }
          else if (compatible($1->exp_type, $3->exp_type))
          {
               $$ = new op_binary_astnode(INT_TYPE, GE_OP_, _INT, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary >=, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     ;

additive_expression: 
     multiplicative_expression
     {
	     $$ = $1;
     }
     | additive_expression '+' multiplicative_expression
     {
          if ($1->exp_type == FLOAT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, PLUS_, _FLOAT, $1, $3);
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, PLUS_, _FLOAT, float_cast($1), $3);
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, PLUS_, _FLOAT, $1, float_cast($3));
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, PLUS_, _INT, $1, $3);
          }
          else if (($1->exp_type == INT_TYPE && (isPointer($3->exp_type) || isArray($3->exp_type))))
          {
               $$ = new op_binary_astnode($3->exp_type, PLUS_, _INT, $1, $3);
          }
          else if (((isPointer($1->exp_type) || isArray($1->exp_type)) && $3->exp_type == INT_TYPE))
          {
               $$ = new op_binary_astnode($1->exp_type, PLUS_, _INT, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary +, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     | additive_expression '-' multiplicative_expression
     {
          if ($1->exp_type == FLOAT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, MINUS_, _FLOAT, $1, $3);
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, MINUS_, _FLOAT, float_cast($1), $3);
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, MINUS_, _FLOAT, $1, float_cast($3));
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, MINUS_, _INT, $1, $3);
          }
          else if (((isPointer($1->exp_type) || isArray($1->exp_type)) && $3->exp_type == INT_TYPE))
          {
               $$ = new op_binary_astnode($1->exp_type, MINUS_, _INT, $1, $3);
          }
          else if (compatible($1->exp_type, $3->exp_type))
          {
               $$ = new op_binary_astnode(INT_TYPE, MINUS_, _INT, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary -, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     ;

multiplicative_expression: 
     unary_expression
     {
	     $$ = $1;
     }
     | multiplicative_expression '*' unary_expression
     {
          if ($1->exp_type == FLOAT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, MULT_, _FLOAT, $1, $3);
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, MULT_, _FLOAT, float_cast($1), $3);
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, MULT_, _FLOAT, $1, float_cast($3));
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, MULT_, _INT, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary *, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     | multiplicative_expression '/' unary_expression
     {
          if ($1->exp_type == FLOAT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, DIV_, _FLOAT, $1, $3);
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == FLOAT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, DIV_, _FLOAT, float_cast($1), $3);
          }
          else if ($1->exp_type == FLOAT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(FLOAT_TYPE, DIV_, _FLOAT, $1, float_cast($3));
          }
          else if ($1->exp_type == INT_TYPE && $3->exp_type == INT_TYPE)
          {
               $$ = new op_binary_astnode(INT_TYPE, DIV_, _INT, $1, $3);
          }
          else
          {
               error(@$, "Invalid operands types for binary /, \"" + $1->exp_type + "\" and \"" + $3->exp_type + "\"");
          }
     }
     ;

unary_expression: 
     postfix_expression
     {
	     $$ = $1;
     }
     | unary_operator unary_expression
     {
          if ($1 == ADDRESS)
          {
               if ($2->l_value == 0)
               {
                    error(@$, "Operand of & should have lvalue");
               }
               else
               {
                    $$ = new op_unary_astnode(pointer($2->exp_type), ADDRESS, $2);
               }
          }
          else if ($1 == DEREF)
          {
               if (!isArray($2->exp_type) && !isPointer($2->exp_type))
               {
                    error(@$, "Invalid operand type \"" + $2->exp_type + "\" of unary *");
               }
               else if (deref($2->exp_type) == VOID_TYPE)
               {
                    error(@$, "Dereferencing \"void *\" pointer");
               }
               else
               {
                    $$ = new op_unary_astnode(deref($2->exp_type), DEREF, $2);
               }
          }
          else if ($1 == UMINUS)
          {
               if ($2->exp_type != INT_TYPE && $2->exp_type != FLOAT_TYPE)
               {
                    error(@$, "Operand of unary - should be an int or a float");
               }
               else
               {
                    $$ = new op_unary_astnode($2->exp_type, UMINUS, $2);
               }
          }
          else if ($1 == NOT)
          {
               if ($2->exp_type != INT_TYPE && $2->exp_type != FLOAT_TYPE && !isArray($2->exp_type) && !isPointer($2->exp_type))
               {
                    error(@$, "Operand of unary ! should be an int or a float or a pointer");
               }
               else
               {
                    $$ = new op_unary_astnode(INT_TYPE, NOT, $2);
               }
          }
          else
          {
               error(@$, "Not Possible");
          }
     }
     ;

postfix_expression: 
     primary_expression
     {
	     $$ = $1;
     }
     | postfix_expression '[' expression ']'
     {
          if ($3->exp_type != INT_TYPE)
          {
               error(@$, "Array subscript is not an integer");
          }
          else
          {
               if (!isArray($1->exp_type) && !isPointer($1->exp_type))
               {
                    error(@$, "Subscripted value is neither array nor pointer");
               }
               else if (deref($1->exp_type) == VOID_TYPE)
               {
                    error(@$, "Dereferencing \"void *\" pointer");
               }
               else
               {
                    $$ = new arrayref_astnode(deref($1->exp_type), $1, $3);
               }
          }
     }
     | IDENTIFIER '(' ')'
     {
          if (present(gst, $1) || (entity_type.first == FUN_TYPE && entity_type.second->name == $1))
          {
               if (func_params[$1].size() == 0) 
               {
                    if (present(gst, $1))
                    {
                         $$ = new funcall_astnode(gst->entries[$1]->type, new identifier_astnode(gst->entries[$1]->type, $1), std::vector<exp_astnode*> ());
                    }
                    else
                    {
                         $$ = new funcall_astnode(entity_type.second->type->type, new identifier_astnode(entity_type.second->type->type, $1), std::vector<exp_astnode*> ());
                    }
               }
               else
               {
                    error(@$, "Function \"" + $1 + "\" called with too few arguments");
               }
          }
          else if (PREDEFINED.find($1) != PREDEFINED.end())
          {
               $$ = new funcall_astnode(PREDEFINED.at($1), new identifier_astnode(PREDEFINED.at($1), $1), std::vector<exp_astnode*> ());
          }
          else
          {
               error(@$, "Function \"" + $1 + "\" is not declared");
          }
     }
     | IDENTIFIER '(' expression_list ')'
     {
          if (present(gst, $1) || (entity_type.first == FUN_TYPE && entity_type.second->name == $1))
          {
               if (func_params[$1].size() == $3.size())
               {
                    for (uint i = 0; i < func_params[$1].size(); i++){
                         if (func_params[$1][i]->type->type == INT_TYPE && $3[i]->exp_type == FLOAT_TYPE)
                         {
                              $3[i] = int_cast($3[i]);
                         }
                         else if (func_params[$1][i]->type->type == FLOAT_TYPE && $3[i]->exp_type == INT_TYPE)
                         {
                              $3[i] = float_cast($3[i]);
                         }
                         else if (func_params[$1][i]->type->type == $3[i]->exp_type)
                         {
                              continue;
                         }
                         else if (func_params[$1][i]->type->type == VOID_POINTER_TYPE && (isArray($3[i]->exp_type) || isPointer($3[i]->exp_type)))
                         {
                              continue;
                         }
                         else if ((isArray(func_params[$1][i]->type->type) || isPointer(func_params[$1][i]->type->type)) && $3[i]->exp_type == VOID_POINTER_TYPE)
                         {
                              continue;
                         }                         
                         else if (compatible(func_params[$1][i]->type->type, $3[i]->exp_type))
                         {
                              continue;
                         }
                         else if (isPointer(func_params[$1][i]->type->type) && $3[i]->astnode_type == INTCONST_ASTNODE && ((intconst_astnode*)$3[i])->value == 0)
                         {
                              continue;
                         }
                         else
                         {
                              error(@$, "Expected \"" + func_params[$1][i]->type->type + "\" but argument is of type \"" + $3[i]->exp_type + "\"");
                         }
                    }
                    if (present(gst, $1))
                    {
                         $$ = new funcall_astnode(gst->entries[$1]->type, new identifier_astnode(gst->entries[$1]->type, $1), $3);
                    }
                    else
                    {
                         $$ = new funcall_astnode(entity_type.second->type->type, new identifier_astnode(entity_type.second->type->type, $1), $3);
                    }
               }
               else if (func_params[$1].size() > $3.size()) 
               {
                    error(@$, "Function \"" + $1 + "\" called with too few arguments");
               }
               else if (func_params[$1].size() < $3.size())
               {
                    error(@$, "Function \"" + $1 + "\" called with too many arguments");
               }
               else
               {
                    error(@$, "Not possible");
               }
          }
          else if (PREDEFINED.find($1) != PREDEFINED.end())
          {
	          $$ = new funcall_astnode(PREDEFINED.at($1), new identifier_astnode(PREDEFINED.at($1), $1), $3);
          }
          else
          {
               error(@$, "Function \"" + $1 + "\" is not declared");
          }
     }
     | postfix_expression '.' IDENTIFIER
     {
          if ($1->exp_type.find(STRUCT_TYPE + " ") == 0 && !(isArray($1->exp_type) || isPointer($1->exp_type)))
          {
               if (gst->entries[$1->exp_type]->symbtab->entries.find($3) != gst->entries[$1->exp_type]->symbtab->entries.end())
               {
                    $$ = new member_astnode(gst->entries[$1->exp_type]->symbtab->entries[$3]->type, $1, new identifier_astnode(gst->entries[$1->exp_type]->symbtab->entries[$3]->type, $3));
               }
               else
               {
                    error(@$, "Struct \"" + $1->exp_type + "\" has no member named \"" + $3 + "\"");
               }
          }
          else
          {
               error(@$, "Left operand of \".\" is not a structure");
          }
     }
     | postfix_expression PTR_OP IDENTIFIER
     {
          if ($1->exp_type.find(STRUCT_TYPE + " ") == 0 && (isArray($1->exp_type) || isPointer($1->exp_type)) && !(isArray(deref($1->exp_type)) || isPointer(deref($1->exp_type))))
          {
               if (gst->entries[deref($1->exp_type)]->symbtab->entries.find($3) != gst->entries[deref($1->exp_type)]->symbtab->entries.end())
               {
                    $$ = new arrow_astnode(gst->entries[deref($1->exp_type)]->symbtab->entries[$3]->type, $1, new identifier_astnode(gst->entries[deref($1->exp_type)]->symbtab->entries[$3]->type, $3));
               }
               else
               {
                    error(@$, "Struct \"" + deref($1->exp_type) + "\" has no member named \"" + $3 + "\"");
               }
          }
          else
          {
               error(@$, "Left operand of \"->\" is not a pointer to or an array of structure");
          }
     }
     | postfix_expression INC_OP
     {
          if ($1->l_value == 0)
          {
               error(@$, "Operand of \"++\" should have lvalue");
          }
          else if ($1->exp_type == INT_TYPE || $1->exp_type == FLOAT_TYPE || isPointer($1->exp_type))
          {
	          $$ = new op_unary_astnode($1->exp_type, PP, $1);
          }
          else
          {
               error(@$, "Operand of \"++\" should be an int, a float or a pointer");
          }
     }
     ;

primary_expression: 
     IDENTIFIER
     {
          if (present(lst, $1))
          {
               if (lst->entries[$1]->scope == PARAM_SCOPE && isArray(lst->entries[$1]->type))
               {
                    $$ = new identifier_astnode(arrayToPointer(lst->entries[$1]->type), $1);
               }
               else
               {
                    $$ = new identifier_astnode(lst->entries[$1]->type, $1);
               }
          }
          else
          {
               error(@$, "Variable \"" + $1 + "\" is not declared");
          }
     }
     | INT_CONSTANT
     {
	     $$ = new intconst_astnode(INT_TYPE, $1);
     }
     | FLOAT_CONSTANT
     {
	     $$ = new floatconst_astnode(FLOAT_TYPE, $1);
     }
     | STRING_LITERAL
     {
	     $$ = new stringconst_astnode(STRING_TYPE, $1);
     }
     | '(' expression ')'
     {
	     $$ = $2;
     }
     ;

expression_list: 
     expression
     {
	     $$.push_back($1);
     }
     | expression_list ',' expression
     {
	     $$ = $1;
          $$.push_back($3);
     }
     ;

unary_operator: 
     '-'
     {
	     $$ = UMINUS;
     }
     | '!'
     {
	     $$ = NOT;
     }
     | '&'
     {
	     $$ = ADDRESS;
     }
     | '*'
     {
	     $$ = DEREF;
     }
     ;

selection_statement: 
     IF '(' expression ')' statement ELSE statement
     {
          if(isScalar($3->exp_type))
          {
               $$ = new if_astnode($3, $5, $7);
          }
          else
          {
               error(@$, "Used \"" + $3->exp_type + "\" where scalar is required");
          }
     }
     ;

iteration_statement: 
     WHILE '(' expression ')' statement
     {
          if(isScalar($3->exp_type))
          {
               $$ = new while_astnode($3, $5);
          }
          else
          {
               error(@$, "Used \"" + $3->exp_type + "\" where scalar is required");
          }
     }
     | FOR '(' assignment_expression ';' expression ';' assignment_expression ')' statement
     {
          if(isScalar($5->exp_type))
          {
               $$ = new for_astnode($3, $5, $7, $9);
          }
          else
          {
               error(@$, "Used \"" + $5->exp_type + "\" where scalar is required");
          }
     }
     ;

declaration_list: 
     declaration
     {
     }
     | declaration_list declaration
     {
     }
     ;

declaration: 
     type_specifier declarator_list ';'
     {
     }
     ;

declarator_list: 
     { type = new type_struct(base_type->type, base_type->size); } declarator
     {          
          if (entity_type.first == STRUCT_TYPE)
          {
               lst->entries.insert({$2->name, new SymbTabEntry($2->name, VAR_TYPE, LOCAL_SCOPE, $2->type->size, offset, $2->type->type, NULL)});
          }
          offset += $2->type->size;
          if (entity_type.first == FUN_TYPE)
          {
               lst->entries.insert({$2->name, new SymbTabEntry($2->name, VAR_TYPE, LOCAL_SCOPE, $2->type->size, -offset, $2->type->type, NULL)});
          }
     }
     | declarator_list ',' { type = new type_struct(base_type->type, base_type->size); } declarator
     {          
          if (entity_type.first == STRUCT_TYPE)
          {
               lst->entries.insert({$4->name, new SymbTabEntry($4->name, VAR_TYPE, LOCAL_SCOPE, $4->type->size, offset, $4->type->type, NULL)});
          }
          offset += $4->type->size;
          if (entity_type.first == FUN_TYPE)
          {
               lst->entries.insert({$4->name, new SymbTabEntry($4->name, VAR_TYPE, LOCAL_SCOPE, $4->type->size, -offset, $4->type->type, NULL)});
          }
     }
     ;
%%
void IPL::Parser::error( const location_type &l, const std::string &err_message )
{
   std::cout << "Error at line " << l.begin.line << ": " << err_message << "\n";
   exit(1);
}


