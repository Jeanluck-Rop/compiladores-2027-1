#include <stdio.h>
#include <string.h>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "semantic/semantic.h"

#define MINIC_EXIT_OK 0       //programa correcto / tokens impresos
#define MINIC_EXIT_INVALID 1  //errores lexicos, sintacticos o semanticos
#define MINIC_EXIT_INTERNAL 2 //uso incorrecto, E/S o memoria

/* Ejecuta el analisis semantico sobre el AST ya construido */
static int
analyze(ASTNode *root)
{
    SemanticAnalyzer analyzer;
    SemanticResult result;

    if (!semantic_analyzer_init(&analyzer)) {
        return MINIC_EXIT_INTERNAL;
    }
    result = semantic_analyze(&analyzer, root);
    semantic_analyzer_destroy(&analyzer);

    switch (result) {
    case SEM_OK:
        printf("Programa semanticamente correcto.\n");
        return MINIC_EXIT_OK;
    case SEM_ERRORS:
        return MINIC_EXIT_INVALID;   //diagnosticos ya enviados a stderr
    default:
        return MINIC_EXIT_INTERNAL;
    }
}

/* Lexer -> parser -> (AST) -> analisis semantico */
static int
compile(FILE *file, int show_ast)
{
    Lexer lexer;
    Parser parser;
    ParseResult result;
    ASTNode *root = NULL;
    int exit_code;

    if (!lexer_init(&lexer, file)) {
        fprintf(stderr, "Error: no se pudo inicializar el lexer.\n");
        return MINIC_EXIT_INTERNAL;
    }

    if (!parser_init(&parser, &lexer)) {
        parser_destroy(&parser);
        lexer_destroy(&lexer);
        return MINIC_EXIT_INTERNAL;
    }

    result = parser_parse_program(&parser, &root);

    switch (result) {
    case PARSE_OK:
        if (show_ast) {
            //Modo depuracion (--ast):
            // comportamiento de la P4, sin  analisis semantico
            printf("AST:\n");
            ast_print(root);
            printf("Programa sintacticamente correcto.\n");
            exit_code = MINIC_EXIT_OK;
        } else {
            exit_code = analyze(root);
        }
        break;
    case PARSE_INVALID:
        exit_code = MINIC_EXIT_INVALID;   //no se corre el semantico
        break;
    default:
        exit_code = MINIC_EXIT_INTERNAL;
        break;
    }

    ast_destroy(root);
    parser_destroy(&parser);
    lexer_destroy(&lexer);
    return exit_code;
}

/* Main del programa */
int
main(int argc,
     char **argv)
{
    const char *program = (argc > 0 && argv[0] != NULL) ? argv[0] : "minic";
    const char *path;
    int print_tokens = 0;
    int show_ast = 0;
    FILE *file;
    int result;

    if (argc == 2 && argv[1][0] != '-') {
        path = argv[1];
    } else if (argc == 3 && strcmp(argv[1], "-t") == 0) {
        print_tokens = 1;
        path = argv[2];
    } else if (argc == 3 && strcmp(argv[1], "--ast") == 0) {
        show_ast = 1;
        path = argv[2];
    } else {
        fprintf(stderr, "Uso: %s <archivo.mc>\n", program);
        return MINIC_EXIT_INTERNAL;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: no se pudo abrir '%s'.\n", path);
        return MINIC_EXIT_INTERNAL;
    }

    //Dependiendo de la bandera, aplicamos el modo lexer o modo parser
    result = print_tokens ? lexer_print_tokens(file) : compile(file, show_ast);
    
    if (fclose(file) != 0 && result == MINIC_EXIT_OK) {
        fprintf(stderr, "Error: no se pudo cerrar '%s'.\n", path);
        return MINIC_EXIT_INTERNAL;
    }
    
    return result;
}
