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


test: test-cli test-unit

# Realizado por nosotros (intentos sin exito) mejorado por Claude

## Definimos test-cli que compila minic y corre todos los casos de prueba
## Corremos cada caso de prueba ejecutando minic sobre cada .mc de inputs/,
##   comparamos su stdout contra el .out correspondiente en expected/,
##   e imprimimos PASS o FAIL por caso (mostrando el diff cuando falla)
## Solo se captura stdout del programa, si minic escribe algo en stderr
##   esto no afecta la comparacion de modo que, diagnosticos y tokens no se mezclan
## Al final imprimimos un resumen y el make falla si hubo algun FAIL
test-cli: $(TARGET)
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

## Definimos test-unit que compila y corre cada archivo de pruebas unitarias
## Corremos cada test_*.c de unit_tests/ compilandolo junto con las fuentes
##   del lexer (sin main.c, ya que cada test define su propio main), e
##   imprimimos FAIL si no compila o si el binario termina con error
## El binario en si decide PASS o FAIL mediante sus propios assert, si algun
##   assert falla el programa aborta con codigo de salida distinto de cero,
##   y el make lo detecta sin necesidad de comparar contra un .out
## Al final imprimimos un resumen y el make falla si hubo algun FAIL
test-unit:
	@mkdir -p $(BUILD_DIR)
	@fail=0; \
	for test_src in $(UNIT_TEST_SOURCES); do \
		name=$$(basename $$test_src .c); \
		bin=$(BUILD_DIR)/$$name; \
		echo "== $$name =="; \
		if $(CC) $(CPPFLAGS) $(CFLAGS) $$test_src $(LEXER_SOURCES) -o $$bin; then \
			if ./$$bin; then \
				: ; \
			else \
				echo "FAIL: $$name"; \
				fail=$$((fail+1)); \
			fi; \
		else \
			echo "FAIL: $$name (no compiló)"; \
			fail=$$((fail+1)); \
		fi; \
	done; \
	echo "----"; \
	if [ $$fail -ne 0 ]; then \
		echo "Unit tests con errores: $$fail"; \
		exit 1; \
	else \
		echo "Todos los unit tests pasaron."; \
	fi

clean:
	$(RM) $(TARGET) $(TARGET).exe
	$(RM) -r $(BUILD_DIR)
