#include "lexer/buffer_manage.h"

#include <stdlib.h>
#include <stdint.h>

/* Reservamos la memoria inicial del buffer */
int
buffer_init(TextBuffer *buffer,
            size_t initial_capacity)
{
    if (initial_capacity == 0)
        initial_capacity = 1;

    buffer->data = malloc(initial_capacity);
    if (buffer->data == NULL) {
        buffer->length = 0;
        buffer->capacity = 0;
        return 0;
    }

    buffer->data[0] = '\0';
    buffer->length = 0;
    buffer->capacity = initial_capacity;
    return 1;
}

/* Agregamos un caracter al buffer, duplicando la capacidad si es necesario */
int
buffer_append(TextBuffer *buffer,
              char c)
{
    if (buffer->length + 2 > buffer->capacity) {
        size_t new_capacity;
        char *new_data;

        if (buffer->capacity > SIZE_MAX / 2)
            return 0;

        new_capacity = buffer->capacity * 2;

        if (new_capacity < buffer->length + 2)
            return 0;

        new_data = realloc(buffer->data, new_capacity);
        if (new_data == NULL)
            return 0;

        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }

    buffer->data[buffer->length] = c;
    buffer->length++;
    buffer->data[buffer->length] = '\0';
    return 1;
}

/* Liberamos la memoria del buffer y lo dejamos en estado seguro */
void
buffer_free(TextBuffer *buffer)
{
    free(buffer->data);
    buffer->data = NULL;
    buffer->length = 0;
    buffer->capacity = 0;
}
