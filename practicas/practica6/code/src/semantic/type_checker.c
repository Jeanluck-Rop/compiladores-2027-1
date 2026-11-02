#include "semantic/type_checker.h"

/* ------------------------------------------------------------------ */
/* Prototipos internos                                                */
/* ------------------------------------------------------------------ */

static int is_undetermined(Type type);

static Type visit_identifier(SemanticAnalyzer *a, ASTNode *node);
static Type visit_integer_literal(SemanticAnalyzer *a, ASTNode *node);
static Type visit_boolean_literal(SemanticAnalyzer *a, ASTNode *node);
static Type visit_unary_expression(SemanticAnalyzer *a, ASTNode *node);
static Type visit_binary_expression(SemanticAnalyzer *a, ASTNode *node);

static Type check_arithmetic(SemanticAnalyzer *a, const ASTNode *node, Type left, Type right);
static Type check_relational(SemanticAnalyzer *a, const ASTNode *node, Type left, Type right);
static Type check_equality(SemanticAnalyzer *a, const ASTNode *node, Type left, Type right);
static Type check_logical(SemanticAnalyzer *a, const ASTNode *node, Type left, Type right);

static const char *binary_operator_symbol(BinaryOperator op);
static void report_operand_mismatch(SemanticAnalyzer *a, const ASTNode *node,
                                    Type expected, Type left, Type right);
static void report_equality_mismatch(SemanticAnalyzer *a, const ASTNode *node,
                                     Type left, Type right);
static void report_unary_mismatch(SemanticAnalyzer *a, const ASTNode *node,
                                  Type expected, Type found);
static void report_initialization_mismatch(SemanticAnalyzer *a, int line, int column,
                                           Type declared, Type found);
static void report_assignment_mismatch(SemanticAnalyzer *a, int line, int column,
                                       Type target, Type found);
static void report_condition_mismatch(SemanticAnalyzer *a, int line, int column,
                                      ConditionKind kind, Type found);

Type
type_from_declared(ASTDeclaredType declared)
{
    return declared == AST_TYPE_BOOL ? TYPE_BOOL : TYPE_INT;
}

Type
type_check_expression(SemanticAnalyzer *a,
                      ASTNode *node)
{
    Type result;

    if (a == NULL || node == NULL || a->had_internal_error)
        return TYPE_UNKNOWN;

    switch (node->type) {
    case AST_IDENTIFIER:
        result = visit_identifier(a, node);
        break;
    case AST_INTEGER_LITERAL:
        result = visit_integer_literal(a, node);
        break;
    case AST_BOOLEAN_LITERAL:
        result = visit_boolean_literal(a, node);
        break;
    case AST_UNARY_EXPRESSION:
        result = visit_unary_expression(a, node);
        break;
    case AST_BINARY_EXPRESSION:
        result = visit_binary_expression(a, node);
        break;
    default:
        return TYPE_UNKNOWN;
    }

    node->inferred_type = result;
    return result;
}

void
type_check_initialization(SemanticAnalyzer *a,
                          ASTNode *declaration)
{
    ASTNode *initializer;
    Type declared;
    Type actual;

    if (a == NULL || declaration == NULL
        || declaration->type != AST_VARIABLE_DECLARATION)
        return;

    initializer = declaration->data.variable_declaration.initializer;
    if (initializer == NULL)
        return;
    
    actual = type_check_expression(a, initializer);
    if (is_undetermined(actual))
        return;

    declared = type_from_declared(declaration->data.variable_declaration.declared_type);
    if (!types_are_compatible(declared, actual))
        report_initialization_mismatch(a, initializer->line, initializer->column,
                                       declared, actual);
}

void
type_check_assignment(SemanticAnalyzer *a,
                      ASTNode *assignment,
                      const Symbol *target)
{
    Type actual;

    if (a == NULL || assignment == NULL || assignment->type != AST_ASSIGNMENT)
        return;

    actual = type_check_expression(a, assignment->data.assignment.value);

    if (target == NULL || is_undetermined(actual))
        return;

    if (!types_are_compatible(target->type, actual))
        report_assignment_mismatch(a, assignment->line, assignment->column,
                                   target->type, actual);
}

void
type_check_condition(SemanticAnalyzer *a,
                     ASTNode *condition,
                     ConditionKind kind)
{
    Type actual;

    if (a == NULL)
        return;

    actual = type_check_expression(a, condition);
    if (is_undetermined(actual))
        return;

    if (actual != TYPE_BOOL)
        report_condition_mismatch(a, condition->line, condition->column, kind, actual);
}


/* ------------------------------------------------------------------ */
/* Utilidades                                                         */
/* ------------------------------------------------------------------ */

/* */
static int
is_undetermined(Type type)
{
    return type == TYPE_ERROR || type == TYPE_UNKNOWN;
}


/* ------------------------------------------------------------------ */
/* Expresiones                                                        */
/* ------------------------------------------------------------------ */

static Type
visit_identifier(SemanticAnalyzer *a,
                 ASTNode *node)
{
    const char *name = node->data.identifier.name;
    Symbol *symbol = symbol_table_lookup(&a->symbol_table, name);

    if (symbol == NULL) {
        semantic_report_undeclared(a, name, node->line, node->column);
        return TYPE_ERROR;
    }
    return symbol->type;
}

static Type
visit_integer_literal(SemanticAnalyzer *a,
                      ASTNode *node)
{
    (void)a;
    (void)node;
    return TYPE_INT;
}

static Type
visit_boolean_literal(SemanticAnalyzer *a,
                      ASTNode *node)
{
    (void)a;
    (void)node;
    return TYPE_BOOL;
}

static Type
visit_unary_expression(SemanticAnalyzer *a,
                       ASTNode *node)
{
    Type operand = type_check_expression(a, node->data.unary.operand);

    if (is_undetermined(operand))
        return TYPE_ERROR;

    if (operand != TYPE_INT) {
        report_unary_mismatch(a, node, TYPE_INT, operand);
        return TYPE_ERROR;
    }
    return TYPE_INT;
}

static Type
visit_binary_expression(SemanticAnalyzer *a,
                        ASTNode *node)
{
    Type left = type_check_expression(a, node->data.binary.left);
    Type right = type_check_expression(a, node->data.binary.right);

    if (is_undetermined(left) || is_undetermined(right))
        return TYPE_ERROR;

    switch (node->data.binary.operator) {
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
        return check_arithmetic(a, node, left, right);
    case OP_LESS:
    case OP_LESS_EQUAL:
    case OP_GREATER:
    case OP_GREATER_EQUAL:
        return check_relational(a, node, left, right);
    case OP_EQUAL:
    case OP_NOT_EQUAL:
        return check_equality(a, node, left, right);
    case OP_AND:
    case OP_OR:
        return check_logical(a, node, left, right);
    }
    return TYPE_ERROR;
}


/* ------------------------------------------------------------------ */
/* Reglas por categoria de operador                                   */
/* ------------------------------------------------------------------ */

static Type
check_arithmetic(SemanticAnalyzer *a,
                 const ASTNode *node,
                 Type left, Type right)
{
    if (left == TYPE_INT && right == TYPE_INT)
        return TYPE_INT;

    report_operand_mismatch(a, node, TYPE_INT, left, right);
    return TYPE_ERROR;
}

static Type
check_relational(SemanticAnalyzer *a,
                 const ASTNode *node,
                 Type left, Type right)
{
    if (left == TYPE_INT && right == TYPE_INT)
        return TYPE_BOOL;

    report_operand_mismatch(a, node, TYPE_INT, left, right);
    return TYPE_ERROR;
}

static Type
check_equality(SemanticAnalyzer *a,
               const ASTNode *node,
               Type left, Type right)
{
    if (left == right)
        return TYPE_BOOL;

    report_equality_mismatch(a, node, left, right);
    return TYPE_ERROR;
}

static Type
check_logical(SemanticAnalyzer *a,
              const ASTNode *node,
              Type left, Type right)
{
    if (left == TYPE_BOOL && right == TYPE_BOOL)
        return TYPE_BOOL;

    report_operand_mismatch(a, node, TYPE_BOOL, left, right);
    return TYPE_ERROR;
}


/* ------------------------------------------------------------------ */
/* Diagnosticos (semantic_report_error: stderr + error_count)         */
/* ------------------------------------------------------------------ */

static const char*
binary_operator_symbol(BinaryOperator op)
{
    switch (op) {
    case OP_ADD:
        return "+";
    case OP_SUBTRACT:
        return "-";
    case OP_MULTIPLY:
        return "*";
    case OP_DIVIDE:
        return "/";
    case OP_LESS:
        return "<";
    case OP_LESS_EQUAL:
        return "<=";
    case OP_GREATER:
        return ">";
    case OP_GREATER_EQUAL:
        return ">=";
    case OP_EQUAL:
        return "==";
    case OP_NOT_EQUAL:
        return "!=";
    case OP_AND:
        return "&&";
    case OP_OR:
        return "||";
    default:
        return "?";
    }
}

static void
report_operand_mismatch(SemanticAnalyzer *a,
                        const ASTNode *node,
                        Type expected,
                        Type left, Type right)
{
    semantic_report_error(a, node->line, node->column,
                          "el operador '%s' requiere operandos\n"
                          "de tipo '%s', pero se encontraron\n"
                          "'%s' y '%s'.",
                          binary_operator_symbol(node->data.binary.operator),
                          type_to_string(expected),
                          type_to_string(left), type_to_string(right));
}

static void
report_equality_mismatch(SemanticAnalyzer *a,
                         const ASTNode *node,
                         Type left, Type right)
{
    semantic_report_error(a, node->line, node->column,
                          "el operador '%s' requiere operandos\n"
                          "del mismo tipo, pero se encontraron\n"
                          "'%s' y '%s'.",
                          binary_operator_symbol(node->data.binary.operator),
                          type_to_string(left), type_to_string(right));
}

static void
report_unary_mismatch(SemanticAnalyzer *a,
                      const ASTNode *node,
                      Type expected, Type found)
{
    semantic_report_error(a, node->line, node->column,
                          "el operador unario '-' requiere\n"
                          "un operando de tipo '%s',\n"
                          "pero se encontro '%s'.",
                          type_to_string(expected), type_to_string(found));
}

static void
report_initialization_mismatch(SemanticAnalyzer *a,
                               int line, int column,
                               Type declared, Type found)
{
    semantic_report_error(a, line, column,
                          "no es posible inicializar una variable\n"
                          "de tipo '%s' con una expresion de tipo '%s'.",
                          type_to_string(declared), type_to_string(found));
}

static void
report_assignment_mismatch(SemanticAnalyzer *a,
                           int line, int column,
                           Type target, Type found)
{
    semantic_report_error(a, line, column,
                          "no es posible asignar un valor de tipo '%s'\n"
                          "a una variable de tipo '%s'.",
                          type_to_string(found), type_to_string(target));
}

static void
report_condition_mismatch(SemanticAnalyzer *a,
                          int line, int column,
                          ConditionKind kind,
                          Type found)
{
    semantic_report_error(a, line, column,
                          "la condicion de una sentencia '%s'\n"
                          "debe ser de tipo 'bool',\n"
                          "pero se encontro '%s'.",
                          kind == CONDITION_IF ? "if" : "while",
                          type_to_string(found));
}
