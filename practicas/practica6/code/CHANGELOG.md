# Changelog


-------------------------------------------


## [Práctica 6] 01-11-2026

### Agregados
- Módulo de tipos: `type.h` / `type.c`.
  - `type.h` define la enumeración `Type` (`TYPE_INT`, `TYPE_BOOL`, `TYPE_ERROR`, `TYPE_UNKNOWN`), que antes
    vivía en `symbol.h`. No incluye ningún otro encabezado del proyecto, para evitar ciclos de inclusión.
  - `type_to_string`, que devuelve el nombre legible de un tipo para los diagnósticos.
  - `types_are_compatible`, que exige igualdad exacta de tipos y nunca acepta `TYPE_ERROR` ni
    `TYPE_UNKNOWN`.
- Módulo del verificador de tipos: `type_checker.h` / `type_checker.c`.
  - `ConditionKind` (`CONDITION_IF`, `CONDITION_WHILE`), para que el mensaje de una condición nombre la
    sentencia correcta.
  - API pública: `type_from_declared`, `type_check_expression`, `type_check_initialization`,
    `type_check_assignment` y `type_check_condition`.
  - Una función de visita por tipo de expresión (`visit_identifier`, `visit_integer_literal`,
    `visit_boolean_literal`, `visit_unary_expression`, `visit_binary_expression`) y una regla por categoría
    de operador (`check_arithmetic`, `check_relational`, `check_equality`, `check_logical`).
  - Un diagnóstico por caso (`report_operand_mismatch`, `report_equality_mismatch`,
    `report_unary_mismatch`, `report_initialization_mismatch`, `report_assignment_mismatch`,
    `report_condition_mismatch`), además de `binary_operator_symbol` e `is_undetermined`.
- Verificación de tipos de literales, identificadores, operadores aritméticos, relacionales, de igualdad y
  lógicos, negación unaria, inicializaciones, asignaciones y condiciones de `if` y `while`. `print` acepta
  `int` y `bool`.
- Propagación de `TYPE_ERROR` para evitar errores en cascada: una expresión con un operando indeterminado
  produce `TYPE_ERROR` sin un diagnóstico nuevo.
- `semantic_report_error` en `semantic.c`: diagnóstico con formato variable (`stdarg` / `vfprintf`) y el
  mismo encabezado `Error semantico [linea:columna]:` de la Práctica 5. Es la única función que cuenta los
  errores de tipos.
- Campo `Type inferred_type` en `ASTNode`: el tipo de cada expresión queda guardado en el árbol. Todo nodo
  nace con `TYPE_UNKNOWN`; solo las expresiones reciben `TYPE_INT`, `TYPE_BOOL` o `TYPE_ERROR`.
- Pruebas públicas de la Práctica 6 en `tests/public/p6/` (24 casos) y target `test-p6` en el Makefile.
- 41 pruebas propias nuevas de verificación de tipos en `tests/unit_tests/semantic/` (prefijo
  `ut_sem_type_`), cada una con `.mc`, `.out`, `.err` y `.code`. Cubren literales, identificadores,
  inicialización, asignación, cada categoría de operador, condiciones, precedencia y asociatividad,
  propagación y cascadas, errores independientes, *shadowing* con tipos y programas completos. La suite
  de `test-unit-semantic` queda en 81 casos.
- Programas de ejemplo `ejemplos/programa_valido.mc` y `ejemplos/programa_invalido.mc`.

### Cambios
- `ast.h` incluye `semantic/type.h` y `ASTNode` guarda `inferred_type`. En `ast.c`, `alloc_node` asigna
  `TYPE_UNKNOWN` explícitamente, porque `calloc` deja el valor `0`, que en la enumeración es `TYPE_INT`.
- `symbol.h` ya no define `Type`: incluye `semantic/type.h`. La estructura `Symbol` no cambió.
- `semantic.h` declara las funciones de diagnóstico que ahora comparten `semantic.c` y `type_checker.c`:
  `semantic_report_error`, `semantic_report_undeclared` y `semantic_report_internal_error`.
- `report_internal_error` y `report_undeclared` dejaron de ser privadas: se renombraron a
  `semantic_report_internal_error` y `semantic_report_undeclared` y se hicieron públicas. El cuerpo de
  ambas no cambió.
- `semantic.c` delega la verificación de tipos al nuevo módulo dentro del mismo recorrido:
  - `visit_variable_declaration` obtiene el tipo del símbolo con `type_from_declared` y, al final, llama a
    `type_check_initialization`.
  - `visit_assignment` resuelve el destino con `symbol_table_lookup`, reporta si no existe y llama a
    `type_check_assignment` con el destino ya resuelto (puede ser `NULL`).
  - `visit_print_statement` llama a `type_check_expression`.
  - `visit_if_statement` y `visit_while_statement` llaman a `type_check_condition` y después recorren sus
    ramas o su cuerpo, sin importar el resultado de la condición.
  - El caso por omisión de `visit_statement` llama a `type_check_expression`.
- `type_from_ast` pasó de `semantic.c` a `type_checker.c` con el nombre `type_from_declared`, porque ahora
  la necesitan los dos módulos.
- Makefile:
  - `SOURCES` incluye `src/semantic/type.c` y `src/semantic/type_checker.c`.
  - Se eliminaron los targets y variables de las pruebas públicas de las Prácticas 1 a 5 (`test-p1` a
    `test-p5`, `PUBLIC_P1_*` a `PUBLIC_P5_*`).
  - Nuevo target `test-p6` y variables `PUBLIC_P6_DIR` y `PUBLIC_P6_SCRIPT`; `test-cli` depende solo de
    `test-p6`.
  - Se conservan `test-unit` (lexer, parser, ast y semantic) y `test-args`.
- Se eliminaron del repositorio las pruebas públicas de las Prácticas 1 a 5 (`tests/public/p1` a
  `tests/public/p5`) para no sobrecargar los archivos de entrega.
- `README.md` reescrito: resalta lo nuevo de esta práctica y menciona brevemente lo anterior.
- Nota de diseño (revisa una predicción del reporte de la Práctica 5): el AST no guarda apuntadores a
  `Symbol`, porque la liberación inmediata de ámbitos los dejaría colgantes. El uso de un identificador se
  resuelve durante el recorrido, con la misma búsqueda visible, y en el árbol solo queda el tipo.

### Mejoras
- Los errores de tipos no se encadenan: un `TYPE_ERROR` (o un `TYPE_UNKNOWN` tras un fallo interno) se
  propaga hacia arriba sin diagnósticos nuevos.
- Los errores independientes no se ocultan: los dos operandos de un operador binario, el valor de una
  asignación sin destino y el inicializador de una redeclaración se analizan siempre, y un error en la
  condición de un `if` o `while` no impide analizar sus ramas o su cuerpo.
- El inicializador de una redeclaración se verifica contra el tipo escrito en esa declaración, y no contra
  el del símbolo original, por lo que no aparece un error de tipos inexistente.
- Un identificador no declarado se reporta una sola vez, aunque aparezca dentro de varias expresiones
  anidadas.
- Una asignación incompatible no modifica el tipo de la variable destino.
- Los diagnósticos de tipos indican el operador o la construcción, el tipo esperado y el tipo encontrado, con
  la posición del operador, del inicializador, del identificador destino o de la condición.
- Los fallos internos siguen distinguiéndose de los errores semánticos: `TYPE_UNKNOWN` evita que, tras un
  fallo de memoria, aparezcan incompatibilidades falsas.
- Memoria verificada con `valgrind` sobre los dos ejemplos: 0 errores y todas las reservas liberadas
  (122 de 122 con `programa_valido.mc` y 103 de 103 con `programa_invalido.mc`).

### Eliminados
- En `semantic.c`:
  - `visit_expression` y su declaración adelantada (`static void visit_expression(SemanticAnalyzer *a,
    ASTNode *node);`). Recorría la expresión solo para buscar identificadores y no calculaba ningún tipo;
    la sustituye `type_check_expression`.
  - `visit_identifier_use`. Su trabajo (buscar el identificador y reportarlo si no existe) lo hace ahora
    `visit_identifier` en `type_checker.c`, que además devuelve el tipo.
  - `type_from_ast`, sustituida por `type_from_declared` en `type_checker.c`.
- La definición de `Type` en `symbol.h` (ahora en `type.h`).
- Las pruebas públicas de las Prácticas 1 a 5 y sus targets del Makefile.
- No se eliminó ninguna de las 40 pruebas propias `ut_sem_*` de la Práctica 5: se comprobó que siguen siendo
  válidas con la verificación de tipos y se conservan sin cambios.

### Sin cambios
- `lexer`, `token`, `keywords`, `buffer_manage`, `parser.c`, `parser.h`, `grammar.c`, `grammar.h` y `main.c`.
- `scope.h` / `scope.c`, `symbol.c`, `symbol_table.h` / `symbol_table.c`.
- En `semantic.c`: `report_redeclaration`, `visit_program`, `visit_block`, `semantic_analyzer_init`,
  `semantic_analyze` y `semantic_analyzer_destroy`, y el formato de los diagnósticos de variable no
  declarada y de redeclaración.
- Códigos de salida (`0`, `1`, `2`), mensaje de éxito `Programa semanticamente correcto.` y bandera `--ast`.
- Las 40 pruebas propias `ut_sem_*` de la Práctica 5 y las pruebas propias del lexer, el parser y el AST.


-------------------------------------------


## [Práctica 5] 23-10-2026

### Agregados
- Módulo `semantic` (análisis semántico, primera etapa), con `symbol`, `scope`, `symbol_table` y `semantic`
  (cada uno con su `.h` en `include/semantic/` y su `.c` en `src/semantic/`).
- `symbol.h` / `symbol.c`: enumeración `Type` (`TYPE_INT`, `TYPE_BOOL`, `TYPE_ERROR`, `TYPE_UNKNOWN`) y
  estructura `Symbol` (nombre copiado, tipo, nivel de ámbito, línea, columna, referencia al nodo de
  declaración y apuntador `next` para encadenar símbolos en un bucket).
- `symbol_create`, que copia el nombre y libera lo reservado si alguna reserva falla, y `symbol_destroy`,
  que acepta `NULL` y nunca libera el nodo del AST.
- `scope.h` / `scope.c`: estructura `Scope` con una tabla hash por ámbito (función `djb2`, 16 buckets
  iniciales, encadenamiento y duplicación del arreglo cuando el factor de carga superaría 0.75), más
  `scope_create`, `scope_destroy`, `scope_lookup` y `scope_insert`.
- `SymbolTableResult` (`ST_OK`, `ST_DUPLICATE`, `ST_INTERNAL_ERROR`, `ST_AT_GLOBAL`) para distinguir éxito,
  duplicado, fallo interno e intento de salir del ámbito global.
- `symbol_table.h` / `symbol_table.c`: estructura `SymbolTable` con la pila de ámbitos (cadena de `parent`)
  y las operaciones `symbol_table_init`, `symbol_table_enter_scope`, `symbol_table_exit_scope`,
  `symbol_table_lookup_current` (solo el ámbito actual), `symbol_table_lookup` (visible, del ámbito actual
  al global), `symbol_table_insert` y `symbol_table_destroy`.
- `semantic.h` / `semantic.c`: estructura `SemanticAnalyzer` (tabla de símbolos, `error_count` y
  `had_internal_error`), `SemanticResult` (`SEM_OK`, `SEM_ERRORS`, `SEM_INTERNAL_ERROR`) y las funciones
  `semantic_analyzer_init`, `semantic_analyze` y `semantic_analyzer_destroy`.
- Recorrido recursivo del AST dividido en funciones `visit_*` (programa, sentencia, bloque, declaración,
  asignación, `print`, `if`, `while`, expresión e identificador).
- Diagnósticos semánticos en `stderr` con el formato `Error semantico [linea:columna]:`, para variable no
  declarada y para redeclaración con referencia a la declaración anterior.
- Detección de variables no declaradas, de redeclaraciones en el mismo ámbito y de usos fuera de ámbito.
- Soporte de *shadowing*: un ámbito interno puede declarar un nombre que existe en uno exterior.
- Acumulación de múltiples errores semánticos en una misma ejecución.
- Bandera `--ast` en `main.c`, que conserva como modo de depuración la salida de la Práctica 4 (`AST:`, el
  árbol y `Programa sintacticamente correcto.`) sin ejecutar el análisis semántico.
- Pruebas públicas de la Práctica 5 en `tests/public/p5/` y target `test-p5` en el Makefile.
- 40 pruebas propias del analizador semántico en `tests/unit_tests/semantic/` (prefijo `ut_sem_`), cada una
  con `.out`, `.err` y `.code`, y target `test-unit-semantic`. Los archivos esperados se generaron con un
  modelo de referencia independiente, validado antes contra seis casos de los ejemplos públicos.
- Contrato de propiedad de memoria del módulo: el símbolo posee su nombre, el ámbito posee sus símbolos, la
  tabla posee la pila de ámbitos y el analizador posee la tabla pero no el AST. La tabla adquiere un símbolo
  solo si `symbol_table_insert` devuelve `ST_OK`.
- Estrategia de liberación inmediata de ámbitos, documentada en `symbol_table.c`: al salir de un bloque se
  destruyen sus símbolos.

### Cambios
- `main.c`: la función `parser` pasó a llamarse `compile` y recibe un indicador `show_ast`. Tras un
  `PARSE_OK` ejecuta el análisis semántico mediante la nueva función `analyze`; ante `PARSE_INVALID` no se
  ejecuta.
- Salida normal de `main.c`: con un programa correcto, `stdout` contiene exactamente
  `Programa semanticamente correcto.`; ya no se imprimen el AST ni el mensaje
  `Programa sintacticamente correcto.`.
- Códigos de salida: `0` programa correcto, `1` errores léxicos, sintácticos o semánticos, `2` errores de
  uso, entrada y salida, o fallo interno de memoria.
- Con errores de cualquier etapa `stdout` queda vacío y los diagnósticos van a `stderr`.
- Orden de liberación en `main.c`: el analizador se destruye dentro de `analyze`; después se libera el AST,
  el parser y el lexer.
- `SOURCES` incluye `src/semantic/symbol.c`, `scope.c`, `symbol_table.c` y `semantic.c`, y `HEADERS`
  incluye también `include/ast/*.h` e `include/semantic/*.h`.
- `test-unit-parser` y `test-unit-ast` invocan `minic` con `--ast` para conservar sus archivos esperados.
- Los scripts de las pruebas públicas de las Prácticas 3 y 4 invocan `minic` con `--ast`.
- `test-cli` incluye `test-p5` y `test-unit` incluye `test-unit-semantic`.
- Las variables `PUBLIC_P5_*` y `UNIT_SEMANTIC_*` indican la ubicación de las pruebas de esta práctica.

### Mejoras
- Una redeclaración no reemplaza al símbolo original: el segundo error sigue apuntando a la primera
  declaración (comprobado con `ut_sem_redeclaration_keeps_original`).
- El inicializador de una redeclaración también se recorre, para detectar otros errores en la misma
  sentencia.
- Los fallos internos (memoria) se distinguen de los errores semánticos y detienen el análisis de forma
  controlada; un fallo de reserva nunca se reporta como redeclaración.
- La tabla hash de cada ámbito crece de forma dinámica, por lo que no hay un límite fijo de símbolos.
- `symbol_table_destroy` es segura tras una inicialización incompleta.

### Eliminados
- La impresión del AST y de `Programa sintacticamente correcto.` en la ejecución normal de `main.c`
  (el AST se conserva solo con `--ast`).

### Sin cambios
- `lexer`, `token`, `keywords`, `buffer_manage`, `parser.c`, `parser.h`, `grammar.c`, `grammar.h`, `ast.c` y
  `ast.h`. La posición de cada declaración ya era la del identificador desde la Práctica 4, por lo que no
  fue necesario modificar el parser.


-------------------------------------------


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


-------------------------------------------


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


-------------------------------------------


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


-------------------------------------------


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
