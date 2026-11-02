# Práctica 6: Verificación de Tipos

## Información general

| Campo        | Información  |
|--------------|--------------|
| Asignatura   | Compiladores |
| No. práctica | 6            |
| Equipo       | Equipo 14    |


## Integrantes

| Nombre completo               | No. de cuenta | Correo electrónico        |
|-------------------------------|---------------|---------------------------|
| Ramírez Juárez María Fernanda | 321204747     | mafer_rj@ciencias.unam.mx |
| Rojo Peña Manuel Ianluck      | 118005762     | ianluckrp@ciencias.unam.mx|


## Estructura del proyecto

```
Practica06_Equipo14/
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
│       ├── symbol_table.h
│       ├── type_checker.h
│       └── type.h
├── Makefile
├── README.md
├── reporte.pdf
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
└── unit_tests
    ├── ast
    │   ├── expected
    │   └── inputs
    ├── lexer
    │   ├── expected
    │   └── inputs
    ├── parser
    │   ├── expected
    │   └── inputs
    └── semantic
        ├── expected
        └── inputs
```

### Módulos nuevos y modificados

| Archivo                       | Responsabilidad                                                                                                      |
|-------------------------------|----------------------------------------------------------------------------------------------------------------------|
| `semantic/type.c`             | **Nuevo.** `type_to_string` y `types_are_compatible` (igualdad exacta; `TYPE_ERROR` y `TYPE_UNKNOWN` nunca son compatibles). |
| `semantic/type_checker.c`     | **Nuevo.** Tipo de cada expresión, reglas por operador, comprobación de inicializaciones, asignaciones y condiciones, y diagnósticos de tipos. |
| `semantic/semantic.c`         | Recorre sentencias, declara símbolos y resuelve nombres; en el mismo recorrido delega la verificación de tipos. Aloja los diagnósticos compartidos. |
| `include/semantic/type.h`     | **Nuevo.** Enumeración `Type`. No incluye ningún otro encabezado del proyecto para evitar ciclos con `ast.h`. |
| `include/semantic/type_checker.h` | **Nuevo.** Interfaz pública del verificador: `type_check_expression`, `type_check_initialization`, `type_check_assignment`, `type_check_condition`, `type_from_declared`. |
| `include/ast/ast.h`           | Agrega `Type inferred_type` a `ASTNode`.                                                                             |
| `include/semantic/symbol.h`   | Usa `Type` de `type.h` en lugar de definirlo.                                                                        |
| `include/semantic/semantic.h` | Agrega `semantic_report_error` (con formato `printf`), `semantic_report_undeclared` y `semantic_report_internal_error`. |

## Requisitos

- GCC con soporte para C11 (se compila con `-std=c11 -Wall -Wextra -Wpedantic`).
- GNU Make.
- Sin dependencias adicionales.

## Compilación y ejecución

```
make              # genera ./minic
make clean        # elimina los archivos generados
make test         # pruebas públicas, propias y de argumentos
./minic <programa>.mc
```

### Salida y códigos de salida

| Situación                                  | `stdout`                            | `stderr`         | Código |
|--------------------------------------------|-------------------------------------|------------------|--------|
| Programa correcto                          | `Programa semanticamente correcto.` | vacío            | `0`    |
| Errores léxicos o sintácticos              | vacío                               | diagnósticos     | `1`    |
| Errores semánticos (**incluye los de tipos**) | vacío                            | diagnósticos     | `1`    |
| Error de uso, apertura, lectura o memoria  | vacío                               | mensaje de error | `2`    |

El análisis semántico (y con él la verificación de tipos) solo se ejecuta si no hubo errores léxicos ni
sintácticos. El modo de depuración `--ast` conserva la salida de la Práctica 4 y no ejecuta el análisis
semántico.

## Sistema de tipos

| Tipo           | Significado                                                                 |
|----------------|-----------------------------------------------------------------------------|
| `int`          | Expresión o variable entera.                                                |
| `bool`         | Expresión o variable booleana.                                              |
| `TYPE_ERROR`   | Expresión afectada por un error semántico; se propaga sin nuevo diagnóstico. |
| `TYPE_UNKNOWN` | Tipo aún no determinado o no aplicable (valor inicial de todo nodo).        |

La compatibilidad es **exacta**: `int` solo con `int` y `bool` solo con `bool`. No hay conversiones
implícitas ni enteros "verdaderos" en las condiciones.

### Reglas

| Construcción               | Operandos requeridos     | Resultado |
|----------------------------|--------------------------|-----------|
| Literal entero / booleano  | —                        | `int` / `bool` |
| Identificador              | —                        | tipo del símbolo visible más cercano (respeta *shadowing*) |
| `+` `-` `*` `/`            | dos `int`                | `int`     |
| `-` unario                 | un `int`                 | `int`     |
| `<` `<=` `>` `>=`          | dos `int`                | `bool`    |
| `==` `!=`                  | dos valores del mismo tipo | `bool`  |
| `&&` `\|\|`                | dos `bool`               | `bool`    |
| Inicialización `T x = e;`  | `e` de tipo `T`          | —         |
| Asignación `x = e;`        | `e` del tipo de `x`      | —         |
| Condición de `if`/`while`  | `bool`                   | —         |
| `print(e);`                | `e` válida, `int` o `bool` | —       |

No se evalúan valores (la división entre cero no se analiza) ni se modifica el tipo declarado de una
variable por una asignación incorrecta.

## Diagnósticos de tipos

Se escriben en `stderr`; la línea empieza en 1 y la columna en 0, igual que en el lexer.

```
Error semantico [1:15]:
no es posible inicializar una variable
de tipo 'int' con una expresion de tipo 'bool'.
```

```
Error semantico [4:0]:
no es posible asignar un valor de tipo 'bool'
a una variable de tipo 'int'.
```

```
Error semantico [1:8]:
el operador '+' requiere operandos
de tipo 'int', pero se encontraron
'int' y 'bool'.
```

```
Error semantico [1:13]:
el operador '==' requiere operandos
del mismo tipo, pero se encontraron
'bool' y 'int'.
```

```
Error semantico [1:6]:
el operador unario '-' requiere
un operando de tipo 'int',
pero se encontro 'bool'.
```

```
Error semantico [2:4]:
la condicion de una sentencia 'if'
debe ser de tipo 'bool',
pero se encontro 'int'.
```

(`while` produce el mismo mensaje con `'while'`.) Los errores de variable no declarada y de redeclaración
conservan el formato de la Práctica 5.

**Criterio de posición:** operadores binarios y unarios, en el operador; inicializaciones, en el nodo del
inicializador (para un literal o identificador es su primer carácter; para una expresión compuesta, su
operador principal); asignaciones, en el identificador destino; condiciones, en el nodo de la expresión
condición.

### Propagación y errores en cascada

- Si algún operando es `TYPE_ERROR`, la expresión completa es `TYPE_ERROR` y **no** se emite otro
  diagnóstico. Lo mismo ocurre en inicializaciones, asignaciones, condiciones y operadores que contengan
  esa expresión.
- Una variable no declarada produce `TYPE_ERROR`: `x + true` con `x` sin declarar reporta solo la
  declaración faltante.
- Los subárboles independientes siguen analizándose: `-true + (false < true)` reporta ambos errores.
- Si el destino de una asignación no existe, se reporta el uso no declarado y se analiza el valor, pero no
  se compara contra un tipo inexistente.
- Un error en la condición no impide analizar el cuerpo del `while` ni ninguna rama del `if`.
- En una redeclaración se reporta el error y se conserva el símbolo original; el inicializador se compara
  con el tipo escrito en esa declaración, no con el del símbolo registrado.

## Decisiones de diseño

- **Un solo recorrido.** `semantic.c` declara símbolos y resuelve nombres, y en la misma visita llama al
  verificador. Cada uso se resuelve contra el ámbito visible en ese punto, y un uso no declarado nunca se
  reporta dos veces.
- **Los tipos se devuelven y se almacenan.** `type_check_expression` devuelve el tipo de la expresión y lo
  guarda en `ASTNode.inferred_type`. Solo las expresiones lo reciben; las sentencias conservan
  `TYPE_UNKNOWN`.
- **El AST no guarda apuntadores a símbolos.** `symbol_table_exit_scope` libera los símbolos de un bloque
  al salir de él, y esos apuntadores quedarían colgantes. El tipo del identificador se obtiene por búsqueda
  visible durante el recorrido.
- **Responsabilidades separadas.** `semantic.c`: sentencias, declaraciones y usos. `type_checker.c`: tipos.
  Dentro del verificador hay una función por tipo de expresión (`visit_*`), una regla por categoría de
  operador (`check_*`) y una función por tipo de diagnóstico (`report_*`).

### Memoria

- `inferred_type` es un valor (`enum`): no añade propiedad ni liberaciones; `ast_destroy` no cambia.
- Los símbolos siguen liberándose al salir de su ámbito; el AST sigue siendo propiedad de `main.c`.
- Para verificar:

```
valgrind --leak-check=full --show-leak-kinds=all ./minic <programa>.mc
```

## Pruebas

| Objetivo                    | Qué ejecuta                                                                                         |
|-----------------------------|-----------------------------------------------------------------------------------------------------|
| `make test-p6`              | **Nuevo.** Suite pública oficial de la práctica (`tests/public/p6`, 24 casos), con su propio script.     |
| `make test-unit-semantic`   | **81 casos propios:** los 40 de la Práctica 5 (`ut_sem_*`, siguen vigentes) y **41 nuevos de tipos** (`ut_sem_type_*`). |
| `make test-unit-lexer`, `test-unit-parser`, `test-unit-ast` | Pruebas propias de prácticas anteriores, sin cambios. |
| `make test-args`            | Validación de la línea de comandos.                                                                 |
| `make test`                 | `test-cli` (`test-p6`), `test-unit` y `test-args`.                                                  |

Cada caso propio compara `stdout` (`.out`), `stderr` (`.err`) y código de salida (`.code`). Los casos de
tipos cubren: literales, identificadores, inicializaciones y asignaciones válidas e inválidas, cada
categoría de operador (válida e inválida), condiciones de `if` y `while`, expresiones anidadas,
precedencia, asociatividad, propagación de `TYPE_ERROR`, errores en cascada, múltiples errores,
*shadowing* con tipos, redeclaración con inicializador, combinación de errores de ámbito y de tipos, y los
programas completos del enunciado. Sus resultados esperados se revisaron contra las reglas y los ejemplos
del enunciado.

## Limitaciones conocidas

- Un anidamiento extremo (del orden de cien mil paréntesis, bloques o `if` anidados) desborda la pila: el
  descenso recursivo, el verificador de tipos, la impresión y la liberación del AST son recursivos. Con
  anidamientos realistas todo funciona.
- La posición de un error de inicialización con expresión compuesta apunta a su operador principal, no a su
  primer carácter (criterio consistente, documentado arriba).
- No se comprueba que una variable esté inicializada antes de usarse, ni se evalúan expresiones; ambas
  cosas quedan fuera de esta práctica.
- Si en una práctica posterior se anota el AST con apuntadores a símbolos, habrá que conservar los ámbitos
  retirados y liberarlos en `symbol_table_destroy`.
