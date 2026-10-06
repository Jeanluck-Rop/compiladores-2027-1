# Práctica 4: Árboles de Sintaxis Abstracta

## Información general

| Campo        | Información  |
|--------------|--------------|
| Asignatura   | Compiladores |
| No. práctica | 4            |
| Equipo       | Equipo 14    |


## Integrantes

| Nombre completo               | No. de cuenta | Correo electrónico        |
|-------------------------------|---------------|---------------------------|
| Ramírez Juárez María Fernanda | 321204747     | mafer_rj@ciencias.unam.mx |
| Rojo Peña Manuel Ianluck      | 118005762     | ianluckrp@ciencias.unam.mx|


## Estructura del proyecto

```
Practica03_Equipo14/
├── CHANGELOG.md
├── include
│   ├── ast
│   │   └── ast.h
│   ├── lexer
│   │   ├── buffer_manage.h
│   │   ├── keywords.h
│   │   ├── lexer.h
│   │   └── token.h
│   └── parser
│       ├── grammar.h
│       ├── parser.h
│       └── parser_internal.h
├── Makefile
├── minic
├── README.md
├── src
│   ├── ast
│   │   └── ast.c
│   ├── lexer
│   │   ├── buffer_manage.c
│   │   ├── keywords.c
│   │   ├── lexer.c
│   │   └── token.c
│   ├── main.c
│   └── parser
│       ├── grammar.c
│       ├── parser.c
│       └── parser_internal.c
└── tests
    ├── public
    │   ├── p1
    │   ├── p2
    |   ├── p3
    │   └── p4
    └── unit_tests
        ├── ast
        ├── lexer
        └── parser
...
```


### Módulos implementados
    
| Archivo o módulo                   | Responsabilidad                                                                                                       |
|------------------------------------|-----------------------------------------------------------------------------------------------------------------------|
| `Makefile`                         | Compilación y ejecución de las pruebas públicas y propias.                                                            |
| `src/main.c`                       | Valida argumentos, abre el archivo, coordina lexer y parser, informa el resultado y el código de salida.              |
| `src/lexer/token.c`                | Construcción, impresión y liberación de tokens.                                                                       |
| `src/lexer/keywords.c`             | Tabla de palabras reservadas y `lookup_keyword`.                                                                      |
| `src/lexer/buffer_manage.c`        | Buffer dinámico para lexemas de longitud arbitraria.                                                                  |
| `src/lexer/lexer.c`                | Interfaz incremental del lexer: entrega un token por llamada.                                                         |
| `src/parser/parser.c`              | Flujo de tokens: lookahead, avance, consumo, diagnósticos y sincronización.                                           |
| `src/parser/grammar.c`             | Una función por no terminal de la gramática de MiniC.                                                                 |
| `src/ast/ast.c`                    | Lista dinámica de nodos, constructores, impresión en formato canónico y liberación recursiva del AST.                 |
| `include/parser/parser.h`          | Interfaz pública del parser (la única que usa `main.c`).                                                              |
| `include/parser/parser_internal.h` | Operaciones auxiliares compartidas entre `parser.c` y `grammar.c`.                                                    |
| `include/parser/grammar.h`         | Punto de entrada de la gramática (`parse_program`).                                                                   |
| `include/ast/ast.h`                | Tipos de nodo, operadores, estructura `ASTNode`, `ASTNodeList` y prototipos de constructores, impresión y liberación. |
## Requisitos

- GCC con soporte para C11.
- GNU Make.
- Dependencias adicionales: ninguna.


## Compilación

```
make
```

Para eliminar los archivos generados:

```
make clean
```

## Pruebas y Ejecución

```
make test
```

```
./minic <programa>.mc
```

Para imprimir la secuencia de tokens (comportamiento de las Prácticas 1 y 2, usado solo en sus pruebas de
regresión):

```
./minic -t <programa>.mc
```

### Salida y códigos de salida

| Situación                                 | `stdout`                             | `stderr`         | Código |
|-------------------------------------------|--------------------------------------|------------------|--------|
| Programa sin errores                      | `Programa sintacticamente correcto.` | vacío            | `0`    |
| Errores léxicos o sintácticos             | vacío                                | diagnósticos     | `1`    |
| Error de uso, apertura, lectura o memoria | vacío                                | mensaje de error | `2`    |

Formato de los diagnósticos:

```
Error sintactico [linea:columna]: se esperaba <elemento>, pero se encontro '<lexema>'.
Error lexico [linea:columna]: caracter invalido '<lexema>'.
```

Cuando el token encontrado es el fin del archivo se imprime `TOKEN_EOF` en lugar de un lexema vacío.

### Propiedad de tokens y lexemas

- Con `LEXER_STATUS_OK`, `lexer_next_token` entrega un token cuyo lexema fue reservado por `token_init`;
  quien lo recibe adquiere su propiedad. Con cualquier otro estado, el token no es válido y no se destruye.
- El parser es dueño de `current` y `previous` mientras `has_current` y `has_previous` estén activos. Al
  avanzar destruye el `previous` anterior y mueve `current` a `previous`, sin copiar el lexema.
- Los tokens `ERROR` se reportan y se destruyen en cuanto se reciben, sin llegar a la gramática.
- `parser_destroy` libera ambos tokens exactamente una vez, aunque haya habido errores.
- El lexer no reserva memoria propia; ni el lexer ni el parser son dueños del archivo, que cierra `main.c`.

### Propiedad de memoria del AST

- El AST guarda copias propias de nombres y lexemas; nunca apuntadores a tokens del lexer o del parser.
  Las cadenas se copian antes de que `parser_advance` destruya el token que las contenía.
- Cada nodo es dueño de sus hijos; `Program` y `Block` son dueños de sus listas, y la raíz es dueña de todo
  el árbol.
- Los constructores `ast_create_*` adquieren los hijos (o la lista) solo si devuelven un nodo válido; si
  devuelven `NULL`, siguen siendo del llamador. `ast_node_list_append` sigue la misma regla.
- `parser_parse_program(parser, &root)` entrega la raíz solo con `PARSE_OK`; con cualquier otro resultado
  `root` es `NULL` y el parser ya liberó los nodos parciales. El llamador libera la raíz con `ast_destroy`,
  que acepta `NULL`.

## Funcionalidades implementadas

- Interfaz incremental del lexer (`lexer_init`, `lexer_next_token`, `lexer_destroy`) que no imprime tokens
  como efecto secundario y distingue un token `ERROR` de un fallo interno (`LexerStatus`).
- Parser descendente recursivo con un token de anticipación, que reconoce:
  - declaraciones (`int x;`, `bool b = true;`), asignaciones e impresión (`print(expr);`);
  - condicionales `if` con `else` opcional, asociado al `if` más cercano;
  - ciclos `while` y bloques `{ ... }`;
  - expresiones con la precedencia y asociatividad publicadas (`||`, `&&`, `==` `!=`, `<` `<=` `>` `>=`,
    `+` `-`, `*` `/`, `-` unario y paréntesis).
- Construcción de un Árbol de Sintaxis Abstracta durante el análisis:
  - nodos para programa, bloque, declaración (con inicializador opcional), asignación, `print`, `if` (con
    `else` opcional), `while`, expresiones binarias y unarias, identificadores y literales enteros y booleanos;
  - asociatividad izquierda de los operadores binarios y asociatividad derecha de la negación, reflejadas en
    la forma del árbol;
  - los paréntesis, los puntos y coma y demás tokens de sintaxis concreta no generan nodos;
  - línea y columna almacenadas en cada nodo, con las posiciones fijadas por el enunciado.
- Impresión del AST en formato canónico (`ast_print`) y liberación recursiva completa (`ast_destroy`).
- Si hay cualquier error léxico o sintáctico, no se entrega ni se imprime ningún AST parcial.
- Diagnósticos sintácticos con línea, columna, elemento esperado y lexema encontrado, en `stderr`.
- Un único diagnóstico léxico por cada token `ERROR`, sin error sintáctico adicional.
- Recuperación por sentencias con los tokens de sincronización publicados, sin ciclos infinitos ni errores
  repetidos, y analizando el archivo hasta el final.
- Verificación de que toda la entrada se consume hasta `TOKEN_EOF`.
- Se conservan todas las funcionalidades léxicas de las Prácticas 1 y 2.

Más detalles sobre diseño e implementación en el reporte de la práctica.

## Pruebas

- `tests/public/p1/` y `tests/public/p2/`: pruebas públicas de las Prácticas 1 y 2, ejecutadas con `-t`
  (`make test-p1`, `make test-p2`).
- `tests/public/p3/`: pruebas públicas de la Práctica 3, ejecutadas con su propio script (YA NO FUNCIONAN EN 
ESTA IMPLEMENTACION)
  `run_public_tests.sh` (`make test-p3`).
- `tests/public/p4/`: pruebas públicas de la Práctica 4, ejecutadas con su propio script
  `run_public_tests.sh` (`make test-p4`).
- `tests/unit_tests/lexer/`: pruebas propias del lexer (`make test-unit-lexer`).
- `tests/unit_tests/parser/`: pruebas propias del parser, enfocadas en la recuperación ante errores
  léxicos y sintácticos (`make test-unit-parser`). Cada caso compara `stdout` (`.out`), `stderr` (`.err`)
  y código de salida (`.code`).
- `tests/unit_tests/ast/`: pruebas propias de la estructura del AST (`make test-unit-ast`).
- `make test-args`: validación de la línea de comandos (sin archivo, dos archivos, archivo inexistente, etc.).
- `make test-cli` corre `test-p1`, `test-p2` y `test-p3`; `make test-unit` corre `test-unit-lexer` y
  `test-unit-parser`; `make test` corre `test-cli`, `test-unit` y `test-args`.

Más detalles de cada caso en la sección "Resultados y pruebas" del reporte.

## Problemas conocidos

- Un anidamiento extremo (del orden de cien mil paréntesis, bloques o `if` anidados) desborda la pila del
  descenso recursivo y termina con `Segmentation fault`. Con niveles de anidamiento realistas el parser
  funciona correctamente.
- Algunos errores pueden producir un segundo diagnóstico derivado. Por ejemplo, en `if x > 0 {` se reporta
  el `(` faltante y, al sincronizar en `x`, también un `=` esperado. El enunciado permite que el número de
  diagnósticos dependa de la recuperación.
- Un caracter no ASCII (por ejemplo `ñ` en UTF-8) produce un token `ERROR` por cada byte.
- No se valida que un entero reconocido quepa en el rango de un tipo numérico concreto, ni hay límite de
  longitud para identificadores o enteros más allá de la memoria disponible.

## Notas de ejecución

- Las pruebas de la practica anterior, debido a la nuestra estructura del código ya no son de utilidad ni aceptan
el nuevo programa.
