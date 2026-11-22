CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -MMD -MP $(shell pkg-config --cflags gtk+-3.0 webkit2gtk-4.1) -I.
LIBS = $(shell pkg-config --libs gtk+-3.0 webkit2gtk-4.1)

SRCS = $(wildcard src/*.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(OBJS:.o=.d)
TARGET = lexlp

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f src/*.o src/*.d $(TARGET)

# Pruebas del subconjunto sintáctico del Entregable 1 y del Entregable 2.
test: $(TARGET)
	@set -e; \
	for f in tests/sintactico/valido_*.lp tests/e2/validos/*.lp; do \
		echo "== $$f =="; \
		./$(TARGET) $$f; \
	done; \
	for f in tests/sintactico/invalido_*.lp tests/e2/invalidos/*.lp; do \
		echo "== $$f =="; \
		if ./$(TARGET) $$f; then \
			echo "ERROR: se esperaba que $$f fuera rechazado"; \
			exit 1; \
		fi; \
	done; \
	echo "Pruebas sintácticas completadas."

# Verifica que el AST se genere y que el JSON sea válido.
test-ast: $(TARGET)
	@./$(TARGET) tests/e2/validos/integral.lp >/dev/null
	@test -s output/ast.txt
	@test -s output/ast.json
	@python3 -c 'import json; json.load(open("output/ast.json"))'
	@echo "AST y JSON válidos."

-include $(DEPS)

.PHONY: all clean test test-ast