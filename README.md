# LexLP - Analizador Léxico (Fase 1)

Este proyecto es la primera fase (Analizador Léxico) para un lenguaje de programación personalizado llamado **LP**.

Originalmente contemplado para ser desarrollado en Rust, el proyecto está completamente implementado en **C++** y cuenta con una interfaz gráfica (GUI) híbrida utilizando tecnologías web.

## Características

- **Motor Léxico (Backend):** Desarrollado en C++ (`src/Lexer.cpp`). Reconoce tokens básicos como:
  - Números enteros (`NUM_INT`)
  - Números decimales (`NUM_DEC`)
  - Identificadores (`ID`)
  - Ignora espacios en blanco.
  - Caracteres no reconocidos se marcan como `DESCONOCIDO`.
- **Tabla de Símbolos:** Integrada en tiempo real. Cuando se detectan identificadores, se registran en una tabla (`src/SymbolTable.cpp`) guardando su posición inicial y contando sus apariciones para evitar duplicados.
- **Interfaz Gráfica (Frontend):** Construida con HTML/CSS/JS (`index.html`). Se incrusta en una ventana nativa gracias a **GTK** y **WebKit2**, permitiendo una comunicación fluida de datos (vía JSON) entre JavaScript y el motor C++.

## Estructura del Proyecto

- `src/main.cpp`: Punto de entrada de la aplicación, configuración de GTK, puente de comunicación JS <-> C++ y carga del `index.html`.
- `src/Lexer.*`: Lógica principal de análisis léxico y recorrido del código fuente.
- `src/Token.*`: Definición de la estructura de un token (tipo, lexema, línea, columna).
- `src/SymbolTable.*`: Estructura para registrar los identificadores encontrados en el código fuente.
- `index.html`: UI que renderiza el editor de código y la tabla de resultados.
- `tests/`: Scripts de prueba (archivos `.lp`) para evaluar diferentes escenarios de entrada.
