CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude
TARGET = minic
SOURCES = src/main.c src/lexer/token.c src/lexer/lexer.c
BUILD_DIR = build

# Directorios de los test, se dividen conforme se van desarrollando las practicas
PUBLIC_P1_INPUT_DIR = tests/public/p1/inputs
PUBLIC_P1_EXPECTED_DIR = tests/public/p1/expected

PUBLIC_P2_INPUT_DIR = tests/public/p2/inputs
PUBLIC_P2_EXPECTED_DIR = tests/public/p2/expected

# Casos de prueba adicionales
UNIT_TEST_INPUT_DIR = tests/unit_tests/inputs
UNIT_TEST_EXPECTED_DIR = tests/unit_tests/expected

.PHONY: all test test-cli test-p1 test-p2 test-unit clean

all: $(TARGET)

$(TARGET): $(SOURCES) include/lexer/token.h include/lexer/lexer.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $(TARGET)

test: test-cli test-unit

## Plantilla reutilizable:
## Recibe directorio de inputs y directorio
##   de expected, y corre el mismo mecanismo de comparacion para ambos.
## Corremos cada caso de prueba ejecutando minic sobre cada .mc del
##   directorio de inputs, comparamos su stdout contra el .out
##   correspondiente en el directorio de expected, e imprimimos PASS o FAIL
##   por caso (mostrando el diff cuando falla)
## Solo se captura stdout del programa, si minic escribe algo en stderr
##   esto no afecta la comparacion de modo que, diagnosticos y tokens no se mezclan
## Al final imprimimos un resumen y el make falla si hubo algun FAIL
define run_test_dir
	@mkdir -p $(BUILD_DIR)
	@pass=0; fail=0; \
	for input in $(wildcard $(1)/*.mc); do \
		name=$$(basename $$input .mc); \
		expected=$(2)/$$name.out; \
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
endef


## test-cli corre ambas entregas publicas (p1 y p2) usando la misma plantilla
test-cli: test-p1 test-p2

## Casos publicos de la Practica 1
test-p1: $(TARGET)
	$(call run_test_dir,$(PUBLIC_P1_INPUT_DIR),$(PUBLIC_P1_EXPECTED_DIR))

## Casos publicos de la Practica 2
test-p2: $(TARGET)
	$(call run_test_dir,$(PUBLIC_P2_INPUT_DIR),$(PUBLIC_P2_EXPECTED_DIR))

## Casos propios adicionales, para cubrir escenarios puntuales en esta y
##   en futuras practicas, con el mismo mecanismo que test-cli
test-unit: $(TARGET)
	$(call run_test_dir,$(UNIT_TEST_INPUT_DIR),$(UNIT_TEST_EXPECTED_DIR))


clean:
	$(RM) $(TARGET) $(TARGET).exe
	$(RM) -r $(BUILD_DIR)
