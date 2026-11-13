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

# Pruebas del subconjunto sintáctico del Entregable 1.
test: $(TARGET)
	@set -e; \
	for f in tests/sintactico/valido_*.lp; do \
		echo "== $$f =="; \
		./$(TARGET) $$f; \
	done; \
	for f in tests/sintactico/invalido_*.lp; do \
		echo "== $$f =="; \
		if ./$(TARGET) $$f; then \
			echo "ERROR: se esperaba que $$f fuera rechazado"; \
			exit 1; \
		fi; \
	done; \
	echo "Pruebas sintácticas completadas."

-include $(DEPS)

.PHONY: all clean test