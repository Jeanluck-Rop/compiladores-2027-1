#ifndef MINIC_TYPE_CHECKER_H
#define MINIC_TYPE_CHECKER_H

#include "ast/ast.h"
#include "semantic/semantic.h"
#include "semantic/symbol.h"
#include "semantic/type.h"

typedef enum {
    CONDITION_IF,
    CONDITION_WHILE
} ConditionKind;

Type type_from_declared(ASTDeclaredType declared);
Type type_check_expression(SemanticAnalyzer *analyzer, ASTNode *node);
void type_check_initialization(SemanticAnalyzer *analyzer, ASTNode *declaration);
void type_check_assignment(SemanticAnalyzer *analyzer, ASTNode *assignment, const Symbol *target);
void type_check_condition(SemanticAnalyzer *analyzer, ASTNode *condition, ConditionKind kind);

#endif
