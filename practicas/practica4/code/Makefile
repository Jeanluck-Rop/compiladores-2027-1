CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
CPPFLAGS = -Iinclude
TARGET = minic
SOURCES = src/main.c \
          src/lexer/token.c src/lexer/keywords.c \
          src/lexer/lexer.c src/lexer/buffer_manage.c \
          src/parser/parser.c src/parser/grammar.c \
          src/ast/ast.c
HEADERS = $(wildcard include/lexer/*.h include/parser/*.h)
BUILD_DIR = build

# Pruebas ayudantia, divididas por practica
PUBLIC_P1_INPUT_DIR = tests/public/p1/inputs
PUBLIC_P1_EXPECTED_DIR = tests/public/p1/expected

PUBLIC_P2_INPUT_DIR = tests/public/p2/inputs
PUBLIC_P2_EXPECTED_DIR = tests/public/p2/expected

# P3 usa un script
PUBLIC_P3_SCRIPT = tests/public/p3/run_public_tests.sh

# P4 usa un script
PUBLIC_P4_SCRIPT = tests/public/p4/run_public_tests.sh

# Pruebas propias
UNIT_LEXER_INPUT_DIR = tests/unit_tests/lexer/inputs
UNIT_LEXER_EXPECTED_DIR = tests/unit_tests/lexer/expected

UNIT_PARSER_INPUT_DIR = tests/unit_tests/parser/inputs
UNIT_PARSER_EXPECTED_DIR = tests/unit_tests/parser/expected

UNIT_AST_INPUT_DIR = tests/unit_tests/ast/inputs
UNIT_AST_EXPECTED_DIR = tests/unit_tests/ast/expected

.PHONY: all test test-cli test-p1 test-p2 test-p3 test-p4 \
        test-unit test-unit-lexer test-unit-parser test-unit-ast test-args clean

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SOURCES) -o $(TARGET)

test: test-cli test-unit test-args

## Plantilla reutilizable:
##   $(1) = directorio de inputs (*.mc)
##   $(2) = directorio de expected
##   $(3) = banderas extra para minic (-t)
##   $(4) = nombre de la suite; las salidas reales se guardan en
##          $(BUILD_DIR)/$(4) para que casos con el mismo nombre en
##          suites distintas no se sobrescriban entre si
## Por cada .mc se ejecuta minic y se compara:
##   - stdout contra <nombre>.out (siempre)
##   - stderr contra <nombre>.err (solo si el archivo existe)
##   - codigo de salida contra <nombre>.code (solo si el archivo existe)
## Asi los casos del lexer, que solo tienen .out, siguen funcionando igual.
## Se imprime PASS o FAIL por caso (con el diff cuando falla), un resumen
##   al final, y make falla si hubo algun FAIL.
define run_test_dir
	@mkdir -p $(BUILD_DIR)/$(4)
	@echo "==== $(4) ===="
	@pass=0; fail=0; \
	for input in $(wildcard $(1)/*.mc); do \
		name=$$(basename $$input .mc); \
		exp=$(2)/$$name; \
		act=$(BUILD_DIR)/$(4)/$$name; \
		./$(TARGET) $(3) $$input > $$act.out 2> $$act.err; \
		echo $$? > $$act.code; \
		ok=1; \
		diff -u $$exp.out $$act.out > /dev/null 2>&1 || ok=0; \
		if [ -f $$exp.err ]; then \
			diff -u $$exp.err $$act.err > /dev/null 2>&1 || ok=0; \
		fi; \
		if [ -f $$exp.code ]; then \
			diff -u $$exp.code $$act.code > /dev/null 2>&1 || ok=0; \
		fi; \
		if [ $$ok -eq 1 ]; then \
			echo "PASS: $$name"; \
			pass=$$((pass+1)); \
		else \
			echo "FAIL: $$name"; \
			diff -u $$exp.out $$act.out; \
			if [ -f $$exp.err ]; then diff -u $$exp.err $$act.err; fi; \
			if [ -f $$exp.code ]; then diff -u $$exp.code $$act.code; fi; \
			fail=$$((fail+1)); \
		fi; \
	done; \
	echo "----"; \
	echo "Total: $$((pass+fail))  Pass: $$pass  Fail: $$fail"; \
	if [ $$fail -ne 0 ]; then exit 1; fi
endef


## ---------- Pruebas ayudante ----------
test-cli: test-p1 test-p2 test-p3 test-p4

## P1 y P2 validan tokens usando el modo -t
test-p1: $(TARGET)
	$(call run_test_dir,$(PUBLIC_P1_INPUT_DIR),$(PUBLIC_P1_EXPECTED_DIR),-t,public-p1)

test-p2: $(TARGET)
	$(call run_test_dir,$(PUBLIC_P2_INPUT_DIR),$(PUBLIC_P2_EXPECTED_DIR),-t,public-p2)

## P3 usa script
test-p3: $(TARGET)
	@mkdir -p $(BUILD_DIR)
	@echo "==== public-p3 ===="
	@sh $(PUBLIC_P3_SCRIPT) | tee $(BUILD_DIR)/public-p3.log
	@grep -q '^Fallidas: 0$$' $(BUILD_DIR)/public-p3.log

## P4 usa script
test-p4: $(TARGET)
	@mkdir -p $(BUILD_DIR)
	@echo "==== public-p4 ===="
	@sh $(PUBLIC_P4_SCRIPT) ./$(TARGET) | tee $(BUILD_DIR)/public-p4.log
	@grep -q '^Fallidas: 0$$' $(BUILD_DIR)/public-p4.log

## ---------- Pruebas propias ----------
test-unit: test-unit-lexer test-unit-parser

## Casos propios del lexer (P1 y P2)
test-unit-lexer: $(TARGET)
	$(call run_test_dir,$(UNIT_LEXER_INPUT_DIR),$(UNIT_LEXER_EXPECTED_DIR),-t,unit-lexer)

## Casos propios del parser (P3)
test-unit-parser: $(TARGET)
	$(call run_test_dir,$(UNIT_PARSER_INPUT_DIR),$(UNIT_PARSER_EXPECTED_DIR),,unit-parser)

## Casos propios del parser (P4)
test-unit-ast: $(TARGET)
	$(call run_test_dir,$(UNIT_AST_INPUT_DIR),$(UNIT_AST_EXPECTED_DIR),,unit-ast)

## Validacion de la linea de comandos.
test-args: $(TARGET)
	@mkdir -p $(BUILD_DIR)/args
	@echo "==== args ===="
	@pass=0; fail=0; \
	out=$(BUILD_DIR)/args/args.out; err=$(BUILD_DIR)/args/args.err; \
	run() { \
		desc="$$1"; want="$$2"; shift 2; \
		./$(TARGET) "$$@" > $$out 2> $$err; code=$$?; \
		if [ $$code -eq $$want ] && [ ! -s $$out ] && [ -s $$err ]; then \
			echo "PASS: $$desc"; pass=$$((pass+1)); \
		else \
			echo "FAIL: $$desc (codigo $$code, esperado $$want)"; \
			cat $$out $$err; fail=$$((fail+1)); \
		fi; \
	}; \
	run "sin archivo" 2; \
	if grep -q '^Uso: .*minic <archivo.mc>$$' $$err; then \
		echo "PASS: mensaje de uso"; pass=$$((pass+1)); \
	else \
		echo "FAIL: mensaje de uso"; cat $$err; fail=$$((fail+1)); \
	fi; \
	run "dos archivos" 2 a.mc b.mc; \
	run "solo -t" 2 -t; \
	run "archivo inexistente" 2 $(BUILD_DIR)/no_existe.mc; \
	run "directorio (no legible)" 2 $(BUILD_DIR); \
	echo "----"; \
	echo "Total: $$((pass+fail))  Pass: $$pass  Fail: $$fail"; \
	if [ $$fail -ne 0 ]; then exit 1; fi

clean:
	$(RM) $(TARGET) $(TARGET).exe
	$(RM) -r $(BUILD_DIR)
 
