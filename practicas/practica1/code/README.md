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
    └── README.md
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

El proyecto cuenta con dos formas de pruebas independientes, que `make test` ejecuta en conjunto:

Pruebas de `tests/public/`, ejecutan el binario `minic` completo sobre cada archivo `.mc` de 
`tests/public/inputs/` y comparan su salida contra `tests/public/expected/*.out`, se ejecutan con:
```
make test-cli
```

En `tests/unit_tests/`, compilan y ejecutan directamente funciones internas del lexer y del sistema de tokens
(por ejemplo `simple_token_type`, `token_init`), sin pasar por el binario completo ni por argumentos de línea de
comandos, se corren con:
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
Corre primero `test-cli` y después `test-unit`. `test-cli` compara la salida de `minic` contra los archivos 
esperados en `tests/public/expected/*.out`, e imprime `PASS`/`FAIL` por cada caso junto con un resumen final. 
`test-unit` compila y ejecuta cada archivo de `tests/unit_tests/`, reportando `FAIL` si no compila o si algún 
`assert` interno falla.

Mientras que, el comando:
```
./minic <programa>.mc
```
Imprime la secuencia de tokens reconocidos en `stdout`, los diagnósticos internos (errores de lectura, memoria,
etc.) se imprimen por separado en `stderr`.

Las salidas de prueba (`.actual`) se generan en `build/`.

Como nota extra, dado que los `archivos.mc` se encuentran en la ruta **tests/public/inputs/**, el comando 
`./minic <programa>.mc` no debe ejecutarse de ese modo a secas, sino con la ruta al archivo de entrada, ejemplo:
```
./minic tests/public/inputs/<programa>.mc
```
A menos que algún archivo *.mc* se encuentre en el directorio actual del ejecutable.
