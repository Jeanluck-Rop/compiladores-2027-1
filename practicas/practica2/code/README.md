# Práctica 2: Analizador léxico completo de MiniC

## Información general

| Campo        | Información  |
|--------------|--------------|
| Asignatura   | Compiladores |
| No. práctica | 2            |
| Equipo       | Equipo 14    |


## Integrantes

| Nombre completo               | No. de cuenta | Correo electrónico        |
|-------------------------------|---------------|---------------------------|
| Ramírez Juárez María Fernanda | 321204747     | mafer_rj@ciencias.unam.mx |
| Rojo Peña Manuel Ianluck      | 118005762     | ianluckrp@ciencias.unam.mx|


## Estructura del proyecto

```
Practica02_Equipo14/
├── CHANGELOG.md
├── include
│   └── lexer
│       ├── buffer_manage.h
│       ├── keywords.h
│       ├── lexer.h
│       └── token.h
├── Makefile
├── README.md
├── reporte.pdf
├── src
│   ├── lexer
│   │   ├── buffer_manage.c
│   │   ├── keywords.c
│   │   ├── lexer.c
│   │   └── token.c
│   └── main.c
└── tests
    ├── public
    │   ├── p1
    │   │   ├── expected
    │   │   └── inputs
    │   ├── p2
    │   │   ├── expected
    │   │   └── inputs
    │   ├── README.md
    │   └── run_public_tests.sh
    ├── README.md
    └── unit_tests
        ├── expected
        │   ├── ut_comment_literal_newline.out
        │   ├── ut_compound_ops_and_comments.out
        │   ├── ut_dense_no_spaces.out
        │   ├── ut_error_recovery_mixed.out
        │   ├── ut_slashes.mc
        │   └── ut_slashes.out
        └── inputs
            ├── ut_comment_literal_newline.mc
            ├── ut_compound_ops_and_comments.mc
            ├── ut_dense_no_spaces.mc
            ├── ut_error_recovery_mixed.mc
            └── ut_slashes.mc
```


### Módulos implementados

| Archivo o módulo | Responsabilidad                 |
|------------------|---------------------------------|
| `Makefile`       | Punto de entrada e integración. |
| `src/lexer.c`    | Punto de entrada e integración. |


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

## Funcionalidades implementadas

- Identificadores (`[a-zA-Z_][a-zA-Z0-9_]*`), distinguidos de palabras reservadas (`int`, `bool`, `if`,
  `else`, `while`, `print`) y de literales booleanos (`true`, `false`).
- Operadores compuestos `==`, `!=`, `<=`, `>=`, `&&`, `||`, con su versión simple correspondiente cuando
  aplica (`=`, `<`, `>`).
- Comentarios de una línea (`//`), ignorados hasta el fin de línea o el fin del archivo.
- Lexemas de longitud arbitraria (enteros e identificadores), gracias a un buffer dinámico.
- Recuperación tras un token `ERROR`: el análisis continúa sin detenerse.

Más detalles sobre diseño e implementación en el reporte de la práctica.

## Pruebas

- `tests/public/p1/`: pruebas públicas de la Práctica 1 (`make test-p1`).
- `tests/public/p2/`: pruebas públicas de la Práctica 2 (`make test-p2`).
- `tests/unit_tests/`: pruebas propias del equipo, enfocadas en casos límite y de error (`make test-unit`).
- `make test-cli` corre `test-p1` y `test-p2` juntos; `make test` corre `test-cli` y `test-unit`.

Más detalles de cada caso en la sección "Resultados y pruebas" del reporte.

## Problemas conocidos

- No se valida que un entero reconocido quepa en el rango de un tipo numérico concreto (por ejemplo,
  overflow de `int`); se considera responsabilidad de una etapa posterior del compilador.
- No hay límite de longitud para identificadores o enteros más allá de la memoria disponible del sistema.

## Notas de ejecución

- `./minic <programa>.mc` imprime la secuencia de tokens en `stdout`; los diagnósticos internos (errores
  de lectura o de memoria) se imprimen por separado en `stderr`.
- Como los archivos de prueba están en rutas como `tests/public/p2/inputs/` o `tests/unit_tests/inputs/`,
  hay que indicar la ruta completa al ejecutar manualmente, por ejemplo:
```
./minic tests/public/p2/inputs/p01_identifiers.mc
```

- Las salidas de prueba (`.actual`) se generan en `build/`, tanto para `test-p1`, `test-p2` como
  `test-unit`.
- Requiere `gcc` con soporte para C11 (`-std=c11`); se compila con `-Wall -Wextra -Wpedantic`.
