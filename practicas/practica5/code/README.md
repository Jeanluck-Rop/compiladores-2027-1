# Práctica 4: Árboles de Sintaxis Abstracta

## Información general

| Campo        | Información  |
|--------------|--------------|
| Asignatura   | Compiladores |
| No. práctica | 5            |
| Equipo       | Equipo 14    |


## Integrantes

| Nombre completo               | No. de cuenta | Correo electrónico        |
|-------------------------------|---------------|---------------------------|
| Ramírez Juárez María Fernanda | 321204747     | mafer_rj@ciencias.unam.mx |
| Rojo Peña Manuel Ianluck      | 118005762     | ianluckrp@ciencias.unam.mx|


## Estructura del proyecto

```
Practica05_Equipo14/
├── CHANGELOG.md
├── include
│   ├── ast
│   │   └── ast.h
│   ├── lexer
│   │   ├── buffer_manage.h
│   │   ├── keywords.h
│   │   ├── lexer.h
│   │   └── token.h
│   ├── parser
│   │   ├── grammar.h
│   │   ├── parser.h
│   │   └── parser_internal.h
│   └── semantic
│       ├── scope.h
│       ├── semantic.h
│       ├── symbol.h
│       └── symbol_table.h
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
│   ├── parser
│   │   ├── grammar.c
│   │   └── parser.c
│   └── semantic
│       ├── scope.c
│       ├── semantic.c
│       ├── symbol.c
│       └── symbol_table.c
└── tests
    ├── public
    │   ├── p1
    │   ├── p2
    |   ├── p3
    |   ├── p4
    │   └── p5
    └── unit_tests
        ├── ast
        ├── lexer
        ├── parser
        └── semantic
...
```

### Módulos implementados

| Archivo o módulo                      | Responsabilidad                                                                                                       |
|---------------------------------------|-----------------------------------------------------------------------------------------------------------------------|
| `Makefile`                            | Compilación y ejecución de las pruebas públicas y propias.                                                            |
| `src/main.c`                          | Valida argumentos, abre el archivo, coordina lexer, parser y analizador semántico, informa el resultado y el código de salida. |
| `src/lexer/token.c`                   | Construcción, impresión y liberación de tokens.                                                                       |
| `src/lexer/keywords.c`                | Tabla de palabras reservadas y `lookup_keyword`.                                                                      |
| `src/lexer/buffer_manage.c`           | Buffer dinámico para lexemas de longitud arbitraria.                                                                  |
| `src/lexer/lexer.c`                   | Interfaz incremental del lexer: entrega un token por llamada.                                                         |
| `src/parser/parser.c`                 | Flujo de tokens: lookahead, avance, consumo, diagnósticos y sincronización.                                           |
| `src/parser/grammar.c`                | Una función por no terminal de la gramática de MiniC.                                                                 |
| `src/ast/ast.c`                       | Lista dinámica de nodos, constructores, impresión en formato canónico y liberación recursiva del AST.                 |
| `src/semantic/symbol.c`               | Creación y destrucción de símbolos (`Symbol`).                                                                        |
| `src/semantic/scope.c`                | Ámbito (`Scope`) con tabla hash dinámica: búsqueda e inserción dentro de un ámbito.                                   |
| `src/semantic/symbol_table.c`         | Pila de ámbitos: entrada y salida de ámbitos, búsqueda local y visible, inserción y destrucción.                      |
| `src/semantic/semantic.c`             | Analizador semántico: recorrido del AST (`visit_*`), reglas de declaración y visibilidad, y diagnósticos.             |
| `include/parser/parser.h`             | Interfaz pública del parser.                                                                                          |
| `include/parser/parser_internal.h`    | Operaciones auxiliares compartidas entre `parser.c` y `grammar.c`.                                                    |
| `include/parser/grammar.h`            | Punto de entrada de la gramática (`parse_program`).                                                                   |
| `include/ast/ast.h`                   | Tipos de nodo, operadores, estructura `ASTNode`, `ASTNodeList` y prototipos de constructores, impresión y liberación. |
| `include/semantic/symbol.h`           | `Type` y `Symbol`; `symbol_create` y `symbol_destroy`.                                                                |
| `include/semantic/scope.h`            | `Scope`, `SymbolTableResult` y operaciones de un ámbito.                                                              |
| `include/semantic/symbol_table.h`     | `SymbolTable` y sus operaciones.                                                                                      |
| `include/semantic/semantic.h`         | `SemanticAnalyzer`, `SemanticResult` y la interfaz del analizador (`init`, `analyze`, `destroy`).                     |

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

Ejecución normal (lexer, parser y análisis semántico):

```
./minic <programa>.mc
```

Para imprimir la secuencia de tokens (comportamiento de las Prácticas 1 y 2, usado solo en sus pruebas de
regresión):

```
./minic -t <programa>.mc
```

Para imprimir el AST (comportamiento de la Práctica 4, solo para pruebas y depuración; **no** ejecuta el
análisis semántico):

```
./minic --ast <programa>.mc
```

### Salida y códigos de salida

| Situación                                 | `stdout`                                | `stderr`         | Código |
|-------------------------------------------|-----------------------------------------|------------------|--------|
| Programa sin errores                      | `Programa semanticamente correcto.`     | vacío            | `0`    |
| Errores léxicos o sintácticos             | vacío                                   | diagnósticos     | `1`    |
| Errores semánticos                        | vacío                                   | diagnósticos     | `1`    |
| Error de uso, apertura, lectura o memoria | vacío                                   | mensaje de error | `2`    |

El análisis semántico solo se ejecuta si no hubo errores léxicos ni sintácticos. En modo normal ya no se
imprimen el AST ni el mensaje `Programa sintacticamente correcto.`; ambos se conservan únicamente con
`--ast`.

Formato de los diagnósticos:

```
Error sintactico [linea:columna]: se esperaba <elemento>, pero se encontro '<lexema>'.
Error lexico [linea:columna]: caracter invalido '<lexema>'.
```

Cuando el token encontrado es el fin del archivo se imprime `TOKEN_EOF` en lugar de un lexema vacío.

Diagnósticos semánticos (la columna comienza en 0, igual que en el lexer):

```
Error semantico [linea:columna]:
la variable '<nombre>' no ha sido declarada.
```

```
Error semantico [linea:columna]:
la variable '<nombre>' ya fue declarada
en este ambito.
Declaracion anterior [linea:columna].
```

Un fallo interno (por ejemplo, memoria insuficiente) no se presenta como error semántico: se informa por
`stderr` y el código de salida es `2`.

### Reglas semánticas implementadas

- Toda variable debe declararse antes de usarse; las sentencias se procesan en el orden del programa y no se
  insertan declaraciones por adelantado.
- Un identificador no puede declararse dos veces en el mismo ámbito, aunque el tipo sea distinto. Una
  redeclaración no reemplaza al símbolo original.
- El ámbito global existe antes del recorrido y cada bloque `{ ... }` crea un ámbito nuevo. Una rama de `if`
  o el cuerpo de un `while` que no sea un bloque no crea ámbito.
- Se permite el *shadowing*: un ámbito interno puede declarar un nombre que existe en uno exterior.
- Al salir de un bloque sus símbolos dejan de ser visibles.
- La variable es visible desde que su declaración se registra, por lo que `int x = x;` no es un error.
- Se reportan todos los errores semánticos del programa; el recorrido no se detiene en el primero.
- **No** se verifican tipos (compatibilidad de asignaciones, operadores o condiciones): corresponde a la
  Práctica 6. El tipo declarado sí se guarda en cada símbolo.

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

### Propiedad de memoria de la tabla de símbolos

- El símbolo (`Symbol`) es dueño de la copia de su nombre. Su campo `declaration` apunta a un nodo del AST
  que **no** le pertenece: `symbol_destroy` nunca lo libera.
- Cada ámbito (`Scope`) es dueño de sus símbolos y de su arreglo de buckets. La tabla (`SymbolTable`) es
  dueña de los ámbitos de la pila vigente, y el analizador es dueño de la tabla pero no del AST, que sigue
  siendo de `main.c`.
- `symbol_table_insert` adquiere el símbolo solo si devuelve `ST_OK`; con `ST_DUPLICATE` o
  `ST_INTERNAL_ERROR`, quien llamó debe liberarlo.
- **Estrategia de liberación: inmediata.** Al salir de un ámbito (`symbol_table_exit_scope`) se destruyen sus
  símbolos. Es válida porque en esta práctica el AST no guarda referencias a símbolos. Si en la siguiente
  práctica se anota el AST con apuntadores a símbolos, los ámbitos retirados deberán conservarse y liberarse
  en `symbol_table_destroy`.
- `symbol_table_enter_scope` no cambia el ámbito actual si falla, y `symbol_table_exit_scope` rechaza salir
  del ámbito global (`ST_AT_GLOBAL`). `symbol_table_destroy` es segura tras una inicialización incompleta.

### Códigos de resultado

| Tipo                | Valor               | Significado                                                |
|---------------------|---------------------|------------------------------------------------------------|
| `SymbolTableResult` | `ST_OK`             | Operación correcta.                                        |
|                     | `ST_DUPLICATE`      | El nombre ya existe en el ámbito actual (error recuperable). |
|                     | `ST_INTERNAL_ERROR` | Fallo de memoria o estado inválido.                        |
|                     | `ST_AT_GLOBAL`      | Se intentó salir del ámbito global.                        |
| `SemanticResult`    | `SEM_OK`            | El programa no tiene errores semánticos.                   |
|                     | `SEM_ERRORS`        | Se detectaron errores semánticos en el programa fuente.    |
|                     | `SEM_INTERNAL_ERROR`| Fallo interno; el análisis se detuvo.                      |

### Tabla de símbolos

- Cada ámbito usa una tabla hash con encadenamiento (función `djb2`). Comienza con 16 buckets y duplica su
  tamaño, redistribuyendo los símbolos, cuando el factor de carga superaría 0.75; no hay un límite fijo de
  símbolos.
- Búsqueda local (`symbol_table_lookup_current`): solo el ámbito actual; se usa para detectar
  redeclaraciones.
- Búsqueda visible (`symbol_table_lookup`): del ámbito actual al global, devolviendo la declaración más
  cercana; se usa para validar usos de identificadores.

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
  - línea y columna almacenadas en cada nodo, con las posiciones fijadas por el enunciado. La declaración
    guarda la posición del identificador, que es la que usan los diagnósticos semánticos.
- Impresión del AST en formato canónico (`ast_print`, modo `--ast`) y liberación recursiva completa
  (`ast_destroy`).
- Si hay cualquier error léxico o sintáctico, no se entrega ni se imprime ningún AST parcial y no se ejecuta
  el análisis semántico.
- Análisis semántico mediante el recorrido del AST (sin volver a leer el archivo ni pedir tokens):
  - tabla de símbolos con pila de ámbitos, ámbito global y un ámbito por bloque;
  - detección de variables no declaradas, de redeclaraciones en el mismo ámbito y de usos fuera de ámbito;
  - *shadowing* entre ámbitos distintos;
  - conservación de nombre, tipo declarado, nivel, línea, columna y nodo de declaración de cada símbolo;
  - acumulación de múltiples errores semánticos sin detenerse en el primero;
  - distinción entre errores semánticos y fallos internos.
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
- `tests/public/p3/` y `tests/public/p4/`: pruebas públicas de las Prácticas 3 y 4, ejecutadas con su propio
  script `run_public_tests.sh` (`make test-p3`, `make test-p4`). Comparan la salida anterior (`AST:`, el
  árbol y `Programa sintacticamente correcto.`), por lo que el script debe invocar `./minic --ast`.
- `tests/public/p5/`: pruebas públicas de la Práctica 5, ejecutadas con su propio script
  `run_public_tests.sh` y con la plantilla del Makefile (`make test-p5`). Doce de sus casos imprimen
  diagnósticos que deben revisarse contra `expected/CASOS.md`.
- `tests/unit_tests/lexer/`: pruebas propias del lexer (`make test-unit-lexer`).
- `tests/unit_tests/parser/`: pruebas propias del parser, enfocadas en la recuperación ante errores
  léxicos y sintácticos (`make test-unit-parser`). Cada caso compara `stdout` (`.out`), `stderr` (`.err`)
  y código de salida (`.code`); se ejecutan con `--ast`.
- `tests/unit_tests/ast/`: pruebas propias de la estructura del AST (`make test-unit-ast`), ejecutadas con
  `--ast`.
- `tests/unit_tests/semantic/`: 40 pruebas propias del analizador semántico, con prefijo `ut_sem_`
  (`make test-unit-semantic`). Cubren declaración y uso, redeclaración (incluida la conservación del símbolo
  original), shadowing, salida de ámbito, bloques hermanos y anidados, `if`/`else`, `while`, múltiples
  errores, orden de sentencias, 300 símbolos (fuerzan el *rehash* y las colisiones) e identificadores largos.
  Cada caso compara `stdout` (`.out`), `stderr` (`.err`) y código de salida (`.code`). Los archivos esperados
  se generaron con un modelo de referencia independiente de la implementación en C.
- `make test-args`: validación de la línea de comandos (sin archivo, dos archivos, archivo inexistente, etc.).
- `make test-cli` corre `test-p1`, `test-p2`, `test-p3`, `test-p4` y `test-p5`; `make test-unit` corre
  `test-unit-lexer`, `test-unit-parser`, `test-unit-ast` y `test-unit-semantic`; `make test` corre
  `test-cli`, `test-unit` y `test-args`.

Más detalles de cada caso en la sección "Resultados y pruebas" del reporte.

Para verificar la memoria:

```
valgrind --leak-check=full --show-leak-kinds=all ./minic <programa>.mc
```

## Problemas conocidos

- Un anidamiento extremo (del orden de cien mil paréntesis, bloques o `if` anidados) desborda la pila del
  descenso recursivo y termina con `Segmentation fault`. El recorrido semántico, la impresión y la
  liberación del AST también son recursivos. Con niveles de anidamiento realistas todo funciona
  correctamente.
- Algunos errores pueden producir un segundo diagnóstico derivado. Por ejemplo, en `if x > 0 {` se reporta
  el `(` faltante y, al sincronizar en `x`, también un `=` esperado. El enunciado permite que el número de
  diagnósticos dependa de la recuperación.
- Un caracter no ASCII (por ejemplo `ñ` en UTF-8) produce un token `ERROR` por cada byte.
- No se valida que un entero reconocido quepa en el rango de un tipo numérico concreto, ni hay límite de
  longitud para identificadores o enteros más allá de la memoria disponible.
- Aún no se verifican tipos ni se comprueba que una variable esté inicializada antes de usarse; ambas
  validaciones quedan para prácticas posteriores.
- La liberación inmediata de ámbitos supone que el AST no conserva apuntadores a símbolos; si se anota el
  árbol con símbolos, hay que cambiar la estrategia (ver "Propiedad de memoria de la tabla de símbolos").

## Notas de ejecución

- Desde la Práctica 5 la salida normal de `./minic` es `Programa semanticamente correcto.` o los
  diagnósticos correspondientes. Las pruebas de las prácticas anteriores que comparan el AST o el mensaje
  sintáctico solo son válidas con `--ast`.
