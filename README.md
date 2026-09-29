### File: `README.md`

```markdown
# Carlos ++ — Analizador Léxico (Lenguaje LP)

[![Lenguaje](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![GUI](https://img.shields.io/badge/GUI-GTK%20%2B%20WebKit2-green.svg)](https://www.gtk.org/)
[![Frontend](https://img.shields.io/badge/Frontend-HTML%20%2F%20CSS%20%2F%20JS-orange.svg)](https://developer.mozilla.org/)

**Carlos ++** es la primera fase (Analizador Léxico) del compilador para el lenguaje de programación personalizado **LP**. 

El proyecto integra un motor de análisis de alto rendimiento escrito en **C++** con una interfaz gráfica (GUI) moderna e interactiva construida en **HTML5, CSS3 y JavaScript**, embebida mediante **GTK+3** y **WebKit2GTK**.

---

## 🚀 Características Principales

- **Motor Léxico Integrado (C++):**
  - Reconocimiento voraz de palabras reservadas, identificadores, constantes enteras y decimales, cadenas con escape y operadores de 1 y 2 símbolos (*lookahead*).
  - Manejo inteligente de cadenas de texto con comillas tipográficas UTF-8 (`“ ”`) normalizadas a comillas estándar (`"`).
  - Tolerancia y recuperación de errores léxicos: reporta todos los fallos en una sola pasada sin detener el proceso.
- **Tabla de Símbolos en Tiempo Real:**
  - Registro de identificadores únicos con control de apariciones, guardando posición inicial y asignando atributos numéricos reutilizables (`<ID, attr>`).
- **Interfaz Gráfica Híbrida (Desktop + Web):**
  - Editor interactivo con cálculo en tiempo real de fila y columna del cursor.
  - Comunicación bidireccional asíncrona mediante IPC (`WebKit JavaScriptBridge` / JSON).
  - Módulos visuales independientes con pestañas para **Tokens**, **Tabla de Símbolos** y **Errores**.
  - Colección de pruebas de código integradas (*samples*).

---

## 📁 Estructura del Proyecto

```text
carlos-plus-plus/
├── src/
│   ├── main.cpp          # Punto de entrada C++, configuración GTK, IPC y WebKit
│   ├── Lexer.h / .cpp    # Algoritmo de recorrido, lógica de tokens y errores
│   ├── Token.h           # Estructuras de datos (Token, LexerError, LexerOutput, to_json)
│   └── SymbolTable.h     # Registro e inserción de identificadores
├── index.html            # Interfaz de usuario (Estructura DOM)
├── style.css             # Estilos oscuros (Dark Mode) y formateo de tokens
├── app.js                # Lógica del cliente, puente IPC y renderizadores
├── tests/                # Scripts de prueba (.lp)
└── README.md             # Documentación del proyecto

```

---

## ⚙️ Arquitectura del Sistema

```text
┌────────────────────────┐        Código Fuente (.lp)       ┌────────────────────────┐
│   Frontend (JS/HTML)   │ ───────────────────────────────► │      Backend (C++)     │
│                        │     ipc.postMessage(code)        │                        │
│  - Editor & Cursor     │                                  │  - Lexer::analyze()    │
│  - Control de Pestañas │ ◄─────────────────────────────── │  - SymbolTable         │
│  - Renderizado DOM     │       showResults(jsonStr)       │  - LexerOutput         │
└────────────────────────┘            JSON                  └────────────────────────┘

```

---

## 🔍 Proceso de Análisis y Reglas Léxicas

El analizador evalúa los caracteres en un orden estrictamente definido para evitar ambigüedades:

| Paso | Patrón / Caracteres | Resultado / Acción |
| --- | --- | --- |
| **1** | Espacios, `\t`, `\r`, `\n` | Se ignoran (`\n` incrementa contador de línea y reinicia columna). |
| **2** | `[a-zA-Z_][a-zA-Z0-9_]*` | Identifica palabra clave (`KW_*`) o registra `ID` en la Tabla de Símbolos. |
| **3** | `"..."` o `“...”` | Genera token `TEXTO`. Si no cierra en la misma línea, genera error de cadena abierta. |
| **4** | Digítos `[0-9]` | Genera `NUM_INT` o `NUM_DEC`. Reporta error de número mal formado si hay múltiples puntos o letras adyacentes. |
| **5** | `/` | Si le sigue `/`, ignora hasta el salto de línea; de lo contrario, genera operador `DIV`. |
| **6** | `=`, `!`, `<`, `>` | Utiliza *lookahead* para reconocer operadores dobles (`==`, `!=`, `<=`, `>=`) o simples. |
| **7** | `&`, `|` | Válidos únicamente en pares (`&&`, `||`). Solitarios se registran como error. |
| **8** | Delimitadores / Operadores | Tokens directos: `+`, `-`, `*`, `%`, `(`, `)`, `[`, `]`, `{`, `}`, `,`, `;`. |
| **9** | Carácter no reconocido | Registra error léxico ("Símbolo no reconocido") y continúa. |

---

## 🏷️ Clasificación de Tokens

| Categoría | Clase CSS | Tipos / Ejemplos |
| --- | --- | --- |
| **Identificadores** | `id-token` | `ID` (`<ID, 0>`, `<ID, 1>`) |
| **Números** | `num-token` | `NUM_INT`, `NUM_DEC` |
| **Palabras Reservadas** | `kw-token` | `KW_INT`, `KW_FLOAT`, `KW_CHAR`, `KW_BOOLEAN`, `KW_VOID`, `KW_IF`, `KW_ELSE`, `KW_FOR`, `KW_WHILE`, `KW_SCANF`, `KW_PRINTLN`, `KW_MAIN`, `KW_RETURN` |
| **Operadores** | `op-token` | `ASSIGN` (`=`), `PLUS`, `MINUS`, `MULT`, `DIV`, `MOD`, `AND`, `OR`, `NOT`, `COMP` |
| **Delimitadores** | `sym-token` | `LPAREN`, `RPAREN`, `LBRACKET`, `RBRACKET`, `LBRACE`, `RBRACE`, `COMMA`, `SEMICOLON` |
| **Cadenas** | `str-token` | `TEXTO` |

---

## 📄 Formato del Protocolo de Salida (JSON)

C++ serializa los resultados utilizando la siguiente estructura:

```json
{
  "tokens": [
    { "type": "INT", "lexeme": "int", "attr": null, "line": 1, "col": 1 },
    { "type": "ID", "lexeme": "x", "attr": 0, "line": 1, "col": 5 },
    { "type": "ASSIGN", "lexeme": "=", "attr": null, "line": 1, "col": 7 },
    { "type": "NUM_DEC", "lexeme": "3.5", "attr": null, "line": 1, "col": 9 },
    { "type": "SEMICOLON", "lexeme": ";", "attr": null, "line": 1, "col": 12 }
  ],
  "errors": [
    { "line": 3, "col": 1, "lexeme": "@", "message": "Simbolo no reconocido" }
  ],
  "symbols": [
    { "pos": 0, "id": "x" }
  ]
}

```

---

## 💻 Ejemplo de Uso en C++

```cpp
#include "Lexer.h"
#include "SymbolTable.h"
#include <iostream>

int main() {
    std::string codigo = "int x = 3.5; // comentario\nprintln(\"ok\");";

    SymbolTable tabla;
    Lexer lexer;
    LexerOutput salida = lexer.analyze(codigo, tabla);

    // Imprime la respuesta JSON formateada para el WebView
    std::cout << salida.to_json() << std::endl;

    return 0;
}

```

---

## 🛠️ Requisitos e Instalación

### Requisitos Previos (Linux / GTK)

* Compilador C++17 (`g++` o `clang++`)
* Librerías de desarrollo GTK3 y WebKit2GTK:

```bash
# Ubuntu / Debian
sudo apt install build-essential libgtk-3-dev libwebkit2gtk-4.0-dev

```

### Compilación y Ejecución

```bash
g++ -std=c++17 src/*.cpp -o carlos_plus_plus `pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.0`
./carlos_plus_plus

```

---

## 📌 Limitaciones Conocidas

* No soporta comentarios multilínea (`/* ... */`).
* Números comenzados por punto (ej. `.5`) se reportan como símbolo no reconocido.
* El conteo de columnas opera a nivel de bytes, por lo que caracteres multibyte UTF-8 dentro de las cadenas pueden desfasar la posición de la columna.

```

```