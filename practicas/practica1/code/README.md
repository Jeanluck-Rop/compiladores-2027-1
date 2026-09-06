# Práctica 1: Infraestructura básica del analizador léxico

## Información general

| Campo        | Información  |
|--------------|--------------|
| Asignatura   | Compiladores |
| No. práctica | 1            |
| Equipo       | Equipo RPRJ  |


## Integrantes

| Nombre completo               | No. de cuenta | Correo electrónico        |
|-------------------------------|---------------|---------------------------|
| Ramírez Juárez María Fernanda | 321204747     | mafer_rj@ciencias.unam.mx |
| Rojo Peña Manuel Ianluck      | 118005762     | ianluckrp@ciencias.unam.mx|


## Estructura del proyecto

```
Practica01_EquipoRPRJ/
├── CHANGELOG.md
├── include
│   ├── lexer
│   ├── lexer.h
│   └── token.h
├── Makefile
├──  mv
├── README.md
├── src
│   ├── main.c
│   └── minic_lexer
│       ├── lexer.c
│       └── token.c
└── tests
    ├── public
    │   ├── expected
    │   ├── inputs
    │   └── README.md
    ├── README.md
    └── unit_tests
        ├── expected
        │   └── p_double_char.out
        └── inputs
            └── p_double_char.mc
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

`**FALTA ESTO**`

- [ ] Lectura del archivo fuente.
- [ ] Sistema de tokens.
- [ ] Seguimiento de línea y columna.
- [ ] Manejo de espacios en blanco.
- [ ] Símbolos simples.
- [ ] Números enteros.
- [ ] Caracteres no reconocidos mediante `ERROR`.
- [ ] Final del archivo mediante `TOKEN_EOF`.

## Pruebas

Simplemente implementamos una prueba para el caso particular de dos simbolos contiguos, como bien se menciona
en las limitantes y/o problemas del reporte, esta prueba busca mostrar que por el momento no tenemos forma de 
detectar los casos para `==`, `<=` y `>=` por ejemplo. Por lo que para probar ambas por separado (las pruebas ya
establecidas y las que el equipo implemento) debemos hacer lo siguiente:

- **`tests/public/`** casos de prueba proporcionados por la práctica, se ejecutan con:
```
make test-cli
```
- **`tests/unit_tests/`** casos adicionales, se ejecutan con:
```
make test-unit
```

## Problemas conocidos

Las limitaciones y problemas que tenemos en cuenta de esta práctica, es que solo se queda en la fase
analizadora y clasificadora del código fuente, no es posible aún darle un significado a esto, aún no podemos
darle la estructura que el análisis sintáctico y semántico otorgan a un compilador. Mucho menos una traducción 
a código objetivo. 

Sin embargo ya se tiene una buena base para implementarlos de manera incremental sin problema alguno. 
Aunque si es necesario notar que una limitante más que se tiene en la práctica 1, es que aún no diferenciamos 
entre caracteres que compartan símbolos, como `=` y `==`, aún no hay una manera específica de diferenciarlos.
Al momento `==` se tomaría como dos `=`.

Si estas no eran las limitantes esperadas por ser demasiado obvias, entonces por parte del equipo
no se conocen problemas adicionales a las limitaciones propias del alcance de la Práctica 1.

## Notas de ejecución

El comando:
```
make test
```
Corre primero `test-cli` y después `test-unit`. Ambos comparan la salida de `minic` contra los
archivos esperados (`tests/public/expected/*.out` y `tests/unit_tests/expected/*.out`
respectivamente), e imprimen `PASS`/`FAIL` por cada caso junto con un resumen final. Si algún
caso de cualquiera de las dos carpetas falla, `make test` termina con código de salida distinto
de cero.

Mientras que, el comando:
```
./minic <programa>.mc
```
Imprime la secuencia de tokens reconocidos en `stdout`, los diagnósticos internos (errores de
lectura, memoria, etc.) se imprimen por separado en `stderr`.

Las salidas de prueba (`.actual`) se generan en `build/`, tanto para `test-cli` como para
`test-unit`.

Como nota extra, dado que los archivos `.mc` se encuentran en rutas como
**tests/public/inputs/** o **tests/unit_tests/inputs/**, el comando `./minic <programa>.mc` no
debe ejecutarse de ese modo a secas, sino con la ruta al archivo de entrada, ejemplo:
```
./minic tests/unit_tests/inputs/p_double_char.mc
```
A menos que algún archivo `.mc` se encuentre en el directorio actual del ejecutable.
