# Obtenido del material compartido por el ayudante de laboratorio
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude
TARGET = minic
SOURCES = src/main.c src/lexer/token.c src/lexer/lexer.c
BUILD_DIR = build

# Definimos los directorios y archivos objetivos para realizar los test
TEST_INPUT_DIR = tests/public/inputs
TEST_EXPECTED_DIR = tests/public/expected
TEST_INPUTS = $(wildcard $(TEST_INPUT_DIR)/*.mc)

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCES) include/lexer/token.h include/lexer/lexer.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $(TARGET)


# Realizado por nosotros (intentos sin exito) mejorado por Claude

## Definimos test que compila minic y corre todos los casos de prueba
## Corremos cada caso de prueba ejecutando minic sobre cada .mc de inputs/,
##   comparamos su stdout contra el .out correspondiente en expected/,
##   e imprimimos PASS o FAIL por caso (mostrando el diff cuando falla)
## Solo se captura stdout del programa, si minic escribe algo en stderr
##   esto no afecta la comparacion de modo que, diagnosticos y tokens no se mezclan
## Al final imprimimos un resumen y el make falla si hubo algun FAIL
test: $(TARGET)
	@mkdir -p $(BUILD_DIR)
	@pass=0; fail=0; \
	for input in $(TEST_INPUTS); do \
		name=$$(basename $$input .mc); \
		expected=$(TEST_EXPECTED_DIR)/$$name.out; \
		actual=$(BUILD_DIR)/$$name.actual; \
		./$(TARGET) $$input > $$actual; \
		if diff -u $$expected $$actual > /dev/null 2>&1; then \
			echo "PASS: $$name"; \
			pass=$$((pass+1)); \
		else \
			echo "FAIL: $$name"; \
			diff -u $$expected $$actual; \
			fail=$$((fail+1)); \
		fi; \
	done; \
	echo "----"; \
	echo "Total: $$((pass+fail))  Pass: $$pass  Fail: $$fail"; \
	if [ $$fail -ne 0 ]; then exit 1; fi

clean:
	$(RM) $(TARGET) $(TARGET).exe
	$(RM) -r $(BUILD_DIR)
