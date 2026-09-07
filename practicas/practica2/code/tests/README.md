# Pruebas del equipo

Las pruebas del equipo se encuentran en el directorio **unit_tests**, desde el directorio donde se enceuntre el
ejecutable de `minic`, se pueden probar como:
```
./minic tests/unit_tests/inputs/p_double_char.mc
```

Y estos casos se ejecutan automáticamente con:
```
make test-unit
```

## Propósito
Esta carpeta contiene casos de prueba adicionales, creados por el equipo para cubrir escenarios específicos que
no necesariamente cubren los que ya vienen incluídos en la práctica por `tests/public/`.

## Tests

| Entrada            | Propósito principal                                                                                                                               |
|--------------------|---------------------------------------------------------------------------------------------------------------------------------------------------|
| `p_double_char.mc` | Operadores de un caracter contiguos (`==`, `<=`, `>=`) se reconocen como dos tokens separados, sin fusionarse en un solo token de dos caracteres. |
