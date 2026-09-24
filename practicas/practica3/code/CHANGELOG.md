# Changelog 

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
