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

Para esta práctica no se implementaron pruebas por parte del equipo, próximamente 

## Problemas conocidos

Las limitaciones y problemas que tenemos en cuenta de esta práctica, es que solo se queda en la fase
analizadora y clasificadora del código fuente, no es posible aún darle un significado a esto, aún no podemos
darle la estructura que el análisis sintáctico y semántico otorgan a un compilador. Mucho menos una traducción 
a código objetivo. Sin embargo ya se tiene una buena base para implementarlos de manera incremental sin 
problema alguno.

Si estas no eran las limitantes esperadas por ser demasiado obvias, entonces por parte del equipo
no se conocen problemas adicionales a las limitaciones propias del alcance de la Práctica 1.

## Notas de ejecución

El comando:
```
make test
```
Corre todos los casos de `tests/public/inputs/*.mc`, compara la salida de `minic` contra los archivos 
esperados en `tests/public/expected/*.out`, e imprime `PASS`/`FAIL` por cada caso junto con un resumen final. 
Si algún caso falla, `make test` termina con código de salida distinto de cero y muestra el `diff` 
correspondiente.

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
