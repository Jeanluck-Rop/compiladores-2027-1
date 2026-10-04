#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"

#include <stdio.h>
#include <string.h>

#define MINIC_EXIT_OK 0       /* programa correcto / tokens impresos */
#define MINIC_EXIT_INVALID 1  /* errores lexicos o sintacticos */
#define MINIC_EXIT_INTERNAL 2 /* uso incorrecto, E/S o memoria */


/* Aplicamoes el parser al archivo y reportamos el resultado general */
static int
parser(FILE *file)
{
    Lexer lexer;
    Parser parser;
    ParseResult result;
    ASTNode *root = NULL;
    
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

    if (result == PARSE_OK) {
        printf("AST:\n");
        ast_print(root);
        printf("Programa sintacticamente correcto.\n");
    }

    ast_destroy(root);          /* acepta NULL */
    parser_destroy(&parser);
    lexer_destroy(&lexer);

    switch (result) {
    case PARSE_OK:
        return MINIC_EXIT_OK;
    case PARSE_INVALID:
        return MINIC_EXIT_INVALID;
    default:
        return MINIC_EXIT_INTERNAL;
    }
}

/* Main del programa */
int
main(int argc,
     char **argv)
{
    const char *program = (argc > 0 && argv[0] != NULL) ? argv[0] : "minic";
    const char *path;
    int print_tokens = 0;
    FILE *file;
    int result;

    if (argc == 2 && strcmp(argv[1], "-t") != 0) {
        path = argv[1];
    } else if (argc == 3 && strcmp(argv[1], "-t") == 0) {
        print_tokens = 1;
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
    result = print_tokens ? lexer_print_tokens(file) : parser(file);
    
    if (fclose(file) != 0 && result == MINIC_EXIT_OK) {
        fprintf(stderr, "Error: no se pudo cerrar '%s'.\n", path);
        return MINIC_EXIT_INTERNAL;
    }
    
    return result;
}
