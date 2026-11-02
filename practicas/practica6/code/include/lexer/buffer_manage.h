#ifndef BUFFER_MANAGE_H
#define BUFFER_MANAGE_H

#include <stddef.h>

/*
 * Buffer dinamico de texto, lo usamos para acumular lexemas
 * (identificadores, palabras reservadas, enteros, etc.)
 * de longitud variable no conocida.
 */
typedef struct {
    char *data;
    size_t length;   /* numero de caracteres almacenados, sin contar '\0' */
    size_t capacity; /* tamano reservado en 'data' */
} TextBuffer;

/*
 * Inicializamos 'buffer' reservando 'initial_capacity' bytes.
 * Si 'initial_capacity' es 0, se usa 1 como minimo.
 * Devuelve 1 en exito, 0 si malloc falla (buffer queda en estado seguro,
 * apto para pasarse a buffer_free sin problema).
 */
int
buffer_init(TextBuffer *buffer, size_t initial_capacity);

/*
 * Agrega el caracter 'c' al final de 'buffer', creciendo la capacidad
 * (duplicandola) si hace falta. Mantiene 'data' siempre terminado en '\0'.
 * Devuelve 1 en exito, 0 si realloc falla o se detecta overflow de
 * capacidad (en ese caso 'buffer' queda sin modificar y sigue siendo
 * valido para buffer_free).
 */
int
buffer_append(TextBuffer *buffer, char c);

/*
 * Libera la memoria de 'buffer' y lo deja en un estado valido
 * y reutilizable (data = NULL, length = capacity = 0).
 * Seguro de llamar aunque buffer_init haya fallado.
 */
void
buffer_free(TextBuffer *buffer);

#endif /* BUFFER_MANAGE_H */
