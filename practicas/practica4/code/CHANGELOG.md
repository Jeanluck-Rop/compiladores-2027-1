# Changelog

## [Práctica 4] 03-10-2026

### Agregados
- Módulo `ast`: `ast.h` y `ast.c`, con los tipos de nodo, los constructores, la impresión del árbol y su
  liberación.
- Enumeraciones `ASTNodeType`, `ASTDeclaredType`, `BinaryOperator` y `UnaryOperator`, y estructura
  `ASTNode` con una unión para los datos de cada tipo de nodo.
- Lista dinámica `ASTNodeList` (`ast_node_list_init`, `ast_node_list_append`, `ast_node_list_destroy`),
  que duplica su capacidad al llenarse.
- Constructores `ast_create_*` para programa, bloque, declaración, asignación, `print`, `if`, `while`,
  expresiones binarias y unarias, identificadores y literales enteros y booleanos.
- Información de línea y columna en cada nodo, con las posiciones fijadas por el enunciado.
- `ast_print`, que imprime el árbol con el formato canónico (dos espacios por nivel, un elemento por línea).
- `ast_destroy`, que libera recursivamente cadenas, hijos, listas y el nodo, y acepta `NULL`.
- `parse_binary_level` en `grammar.c`: función genérica para los niveles de expresiones binarias,
  asociativos por la izquierda, que usa una tabla de pares token/operador.
- `fail_memory` y `copy_string` en `grammar.c`: el primero detiene el análisis si falla un constructor y el
  segundo copia los lexemas que deben sobrevivir al token.
- Contrato de propiedad de memoria: los constructores adquieren los hijos solo si tienen éxito y, si
  devuelven `NULL`, los hijos siguen siendo del llamador.
- Impresión de `AST:`, el árbol y `Programa sintacticamente correcto.` en `main.c`, solo cuando el análisis
  es correcto.
- [CONFIRMAR: pruebas propias del AST en `tests/` y cambios en el Makefile, por ejemplo el target de pruebas
  de la Práctica 4 y la inclusión de `src/ast/` en `SOURCES`.]

### Cambios
- Todas las funciones de `grammar.c` pasaron de `void` a `ASTNode *`: devuelven el nodo que reconocen o
  `NULL` ante un error.
- `parse_program` devuelve la raíz del AST, o `NULL` si hubo cualquier error léxico, sintáctico o interno.
- `parser_parse_program` recibe un parámetro de salida (`ASTNode **out_root`): con `PARSE_OK` entrega la
  raíz y con cualquier otro resultado entrega `NULL`, sin dejar árboles parciales.
- `parser.h` incluye `ast/ast.h`.
- Los seis niveles de expresiones binarias (`||`, `&&`, igualdad, comparación, suma/resta y
  multiplicación/división) se implementan con `parse_binary_level` en lugar de seis ciclos independientes.
- `parse_declaration` y `parse_assignment` copian el nombre del identificador antes de avanzar, porque
  `parser_advance` destruye el token anterior.
- `parse_block` y `parse_program` siguen analizando tras un error para recuperarse, pero descartan la lista
  y devuelven `NULL` si algo falló.
- `parse_primary` devuelve directamente la expresión interna de los paréntesis, sin crear un nodo.
- `main.c`: el mensaje de éxito se imprime después del árbol; se libera el AST antes de destruir el parser
  y el lexer.

### Mejoras
- Ante un error no se entrega ningún árbol parcial: los nodos incompletos se liberan en la función que los
  posee y `stdout` queda vacío.

### Eliminados
- `parse_statement_list`: su ciclo se integró en `parse_program`, que ahora acumula las sentencias en una
  lista.
- El `printf` del mensaje de éxito dentro del `switch` de `main.c`, que lo duplicaba al agregar la
  impresión del AST.


## [Práctica 3] 26-09-2026

### Agregados
- Módulo `parser`: `parser.h` (interfaz pública), `parser_internal.h` (operaciones auxiliares compartidas)
  y `parser.c` (flujo de tokens).
- Módulo `grammar`: `grammar.h` y `grammar.c`, con una función por no terminal de la gramática de MiniC.
- Estructuras `Parser` (lexer, token de anticipación `current`, último token consumido `previous` y
  banderas de error, recuperación y fallo interno) y `ParseResult` (`PARSE_OK`, `PARSE_INVALID`,
  `PARSE_INTERNAL_FAILURE`).
- Enumeración `LexerStatus` (`LEXER_STATUS_OK`, `LEXER_STATUS_IO_ERROR`, `LEXER_STATUS_MEMORY_ERROR`)
  para distinguir un token `ERROR` de un fallo interno del lexer.
- Interfaz incremental del lexer: `lexer_next_token` entrega exactamente un token por llamada y
  `lexer_destroy` deja el lexer en un estado seguro.
- `lexer_print_tokens`, que conserva la impresión de tokens de las Prácticas 1 y 2 sobre la nueva interfaz.
- Opción `-t` en `main.c` para imprimir tokens (solo para las pruebas de regresión del lexer).
- Reconocimiento de declaraciones, asignaciones, `print`, `if`/`else`, `while`, bloques y expresiones con
  precedencia y asociatividad.
- Diagnósticos sintácticos con el formato publicado y diagnósticos léxicos en `stderr`.
- Recuperación por sentencias (`parser_sync`) con `panic_mode` para evitar errores en cascada.
- Verificación del consumo completo de la entrada hasta `TOKEN_EOF`.
- Códigos de salida: `0` programa correcto, `1` errores léxicos o sintácticos, `2` errores de uso,
  apertura, lectura o memoria.
- Pruebas propias del parser en `tests/unit_tests/parser/`, enfocadas en la recuperación ante errores.
- Targets `test-p3` (ejecuta el script público de P3), `test-unit-lexer`, `test-unit-parser` y
  `test-args` en el Makefile.

### Cambios
- `lexer_scan` se reemplazó por `lexer_next_token`: conserva las mismas ramas de clasificación, pero
  termina en cuanto forma un token en lugar de recorrer todo el archivo.
- `lexer_init` pasó a ser pública, recibe directamente el archivo fuente y valida sus argumentos.
- `emit_token` se sustituyó por `make_token`, que solo construye el token en `out`, sin imprimirlo.
- `scan_keyword` y `scan_integer` reciben el `Token *out` que deben llenar y devuelven un `LexerStatus`
  en lugar de imprimir mensajes de error.
- La verificación de `ferror` se movió a la rama de fin de archivo de `lexer_next_token`.
- `main.c` ahora coordina la inicialización, el análisis, la liberación de recursos y el código de salida;
  el mensaje de uso es `Uso: ./minic <archivo.mc>`.
- La interfaz pública del parser se redujo a `parser_init`, `parser_parse_program` y `parser_destroy`; las
  demás operaciones pasaron a `parser_internal.h`.
- La plantilla de pruebas del Makefile recibe banderas para `minic` y un nombre de suite, guarda las
  salidas en `build/<suite>/` y compara además `stderr` (`.err`) y el código de salida (`.code`) cuando
  existen.
- Las pruebas de las Prácticas 1 y 2 se ejecutan ahora con `-t`.
- Las pruebas propias se reorganizaron en `tests/unit_tests/lexer/` y `tests/unit_tests/parser/`.
- `SOURCES` incluye los archivos del parser, y los encabezados se obtienen con `wildcard`.

### Mejoras
- `skip_comment_content` ahora también se detiene en `\r`; antes, con finales de línea que usan solo
  `\r`, un comentario absorbía las líneas siguientes.
- Guarda `c != '\0'` antes de `strchr` en `lexer_next_token`, que de otro modo clasificaba un byte nulo
  como operador.

### Eliminados
- `lexer_scan` y `emit_token`.
- La impresión de tokens como efecto secundario de solicitar un token.
- Copia sobrante de `ut_slashes.mc` en el directorio `expected/` de las pruebas del lexer.
 

## [Práctica 2] 11-09-2026

### Agregados
- Reconocimiento de identificadores (`[a-zA-Z_][a-zA-Z0-9_]*`).
- Distinción entre identificadores, palabras reservadas y literales booleanos mediante una tabla de
  palabras clave (`keywords.h` / `keywords.c`, `lookup_keyword`).
- Reconocimiento de operadores compuestos: `==`, `!=`, `<=`, `>=`, `&&`, `||` (`scan_operator`, con
  anticipación de un caracter).
- Reconocimiento e ignorado de comentarios de una línea (`//`) (`skip_comment_content`).
- Buffer dinámico de texto para lexemas de longitud arbitraria (`buffer_manage.h` / `buffer_manage.c`,
  `TextBuffer`).
- Estructura `Lexer` (`lexer.h`) que encapsula archivo, posición y lookahead de un caracter (`current`).
- Función auxiliar `emit_token` para construir, imprimir y liberar un token en un solo paso.
- Batería propia de pruebas unitarias en `tests/unit_tests/`, enfocada en casos límite y de error.
- Separación de las pruebas públicas en `tests/public/p1/` y `tests/public/p2/`, con nuevos targets
  `test-p1`, `test-p2`, `test-cli` y `test-unit` en el Makefile.

### Cambios
- `simple_token_type` se renombró y redujo a `scan_simple`: ya no incluye `=`, `<`, `>`, `/`, ahora
  resueltos por `scan_operator` y por la detección de comentarios.
- `advance_position` se fusionó dentro de `lexer_advance`, como método de la estructura `Lexer`.
- El ciclo principal de `lexer_scan` pasó de leer directamente con `fgetc` a usar
  `lexer_check` (mirar) / `lexer_advance` (consumir).
- Enteros e identificadores ahora se acumulan en un `TextBuffer` dinámico en vez de un arreglo local de
  tamaño fijo.

### Mejoras
- Buffer de tamaño fijo (64 bytes) para enteros, que producía no permitía números o cadenas muy largas.
- Conteo duplicado de columna al regresar un caracter con `ungetc` sin revertir su avance de posición.
- Desfase constante de una columna en la posición reportada de cada token.

### Eliminados
- Prueba unitaria `p_double_char` de la Práctica 1 (verificaba que `==` se leyera como dos `ASSIGN`;
  ese comportamiento ya no es correcto en la Práctica 2).


## [Práctica 1] 05-09-2026

### Implementados
- Lectura del archivo fuente vía argumentos de línea de comandos.
- Sistema de tokens (`Token`, `TokenType`, `token_init` / `token_print` / `token_destroy`).
- Seguimiento de línea y columna, incluyendo `\r`, `\n` y `\r\n` como un único salto de línea.
- Manejo de espacios en blanco.
- Reconocimiento de símbolos simples de un caracter (`+ - * / = < > ( ) { } ;`).
- Reconocimiento de números enteros sin signo (`[0-9]+`).
- Generación de tokens `ERROR` para caracteres no reconocidos.
- Generación de `TOKEN_EOF` al final del archivo.
- Pruebas públicas (`tests/public`) y unitarias (`tests/unit_tests`) en el Makefile.
