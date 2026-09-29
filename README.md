```markdown
# Carlos ++ — Analizador Léxico (Lenguaje LP)

[![Lenguaje](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![GUI](https://img.shields.io/badge/GUI-GTK%20%2B%20WebKit2-green.svg)](https://www.gtk.org/)
[![Frontend](https://img.shields.io/badge/Frontend-HTML%20%2F%20CSS%20%2F%20JS-orange.svg)](https://developer.mozilla.org/)

**Carlos ++** es la primera fase (Analizador Léxico) del compilador para el lenguaje de programación personalizado **LP**.

El proyecto integra un motor de análisis léxico de alto rendimiento desarrollado en **C++** con una interfaz gráfica (GUI) híbrida e interactiva. El puente de comunicación (*bridge* IPC) se realiza mediante `index.html` utilizando la cabecera `webview.h` (GTK / WebKit2), el cual enlaza y renderiza la vista enriquecida construida en `lexer_ui.html`, `lexer_ui.css` y `lexer_ui.js`.


## 🚀 Características Principales

- **Motor Léxico (Backend C++):**
  - Reconocimiento voraz (*greedy matching*) de palabras clave, identificadores, números enteros (`NUM_INT`), decimales (`NUM_DEC`), cadenas de texto y delimitadores.
  - Soporte de *lookahead* de 1 carácter para distinción de operadores simples y dobles (`=`, `==`, `!`, `!=`, `/`, `//`, etc.).
  - Normalización automática de comillas tipográficas UTF-8 (`“ ”`) a comillas estándar (`"`).
  - Estrategia de **recuperación de errores**: registra todos los fallos léxicos en una sola pasada sin detener el proceso.
- **Tabla de Símbolos en Tiempo Real:**
  - Registro de identificadores únicos asignando atributos posicionales reusables (`<ID, attr>`) y conteo de referencias.
- **Generación de Reportes en Disco (`output/`):**
  - Exportación automática de resultados a archivos de texto plano: `tokens.txt`, `token.txt`, `tabla_simbolos.txt` y `errores.txt`.
- **Interfaz Gráfica Híbrida:**
  - `index.html`: Punto de encolación e integración IPC nativa con C++ vía `webview.h`.
  - `lexer_ui.html`, `lexer_ui.css` y `lexer_ui.js`: Vista interactiva con editor de código, contador de líneas/columnas en tiempo real, pestañas de navegación y temizado de tokens.
- **Suite de Pruebas (.lp):**
  - Scripts predefinidos en `tests/` para evaluar casos básicos, cobertura completa, casos límite y detección de errores.

---

## 📁 Estructura del Proyecto

```text
.
├── argo.toml               # Configuración del proyecto
├── index.html              # Archivo principal de encolación e integración IPC C++ (WebView)
├── lexer_ui.html           # Interfaz gráfica (Estructura DOM de la UI)
├── lexer_ui.css            # Estilos visuales (Dark Mode y colores por tipo de token)
├── lexer_ui.js             # Lógica cliente, formateo DOM e interacción con IPC
├── lexlp                   # Ejecutable binario compilado
├── Makefile                # Script de compilación automatizado
├── webview.h               # Cabecera para el enlace nativo WebView / GTK
├── README.md               # Documentación del proyecto
├── output/                 # Reportes generados en texto plano
│   ├── errores.txt
│   ├── tabla_simbolos.txt
│   ├── tokens.txt
│   └── token.txt
├── src/                    # Código fuente del motor C++
│   ├── Lexer.h / .cpp      # Algoritmo de recorrido y reconocimiento de tokens
│   ├── SymbolTable.h / .cpp# Estructura e inserción en la tabla de símbolos
│   ├── Token.h / .cpp      # Definición de estructuras y serialización a JSON
│   └── main.cpp            # Punto de entrada C++, servidor IPC y carga del WebView
├── tests/                  # Casos de prueba en lenguaje LP
│   ├── prueba_basica.lp
│   ├── prueba_completa.lp
│   ├── prueba_errores.lp
│   └── prueba_limite.lp
└── ui/                     # Recursos auxiliares / fallback
    ├── app.js
    ├── index.html
    └── style.css



---

## ⚙️ Arquitectura del Sistema

```text
┌───────────────────────────────┐                             ┌───────────────────────────────┐
│     Interfaz de Usuario       │     Código fuente (.lp)     │         Backend C++           │
│                               │ ──────────────────────────► │                               │
│  - lexer_ui.html (Vista UI)   │    postMessage(code)        │  - main.cpp (GTK/WebView)     │
│  - lexer_ui.js (Lógica UI)    │                             │  - Lexer::analyze()           │
│  - index.html (Encolación)    │ ◄────────────────────────── │  - SymbolTable                │
└───────────────────────────────┘     showResults(jsonStr)    │  - Escritura en output/*.txt  │
                                              JSON            └───────────────────────────────┘

```

---

## 🔍 Reglas Léxicas y Orden de Reconocimiento

El lexer procesa el texto fuente carácter por carácter respetando la siguiente jerarquía:

| Paso | Expresión / Caracteres | Acción / Token Generado |
| --- | --- | --- |
| **1** | `\n`, `\t`, `\r`, `' '` | Se ignoran. `\n` incrementa `line` y reinicia `col`. |
| **2** | `[a-zA-Z_][a-zA-Z0-9_]*` | Evalúa si es Palabra Reservada (`KW_*`); si no, genera `ID` y lo registra en la Tabla de Símbolos. |
| **3** | `"` o `“ ”` | Lee la cadena hasta cerrarla → Token `TEXTO`. Si llega a fin de línea sin cerrar → Error léxico. |
| **4** | Digítos `[0-9]` | Reconoce `NUM_INT` o `NUM_DEC`. Si contiene múltiples puntos o letras adyacentes → Error "Número mal formado". |
| **5** | `/` | Si le sigue `/` es comentario de línea (se descarta); de lo contrario es `DIV` (`/`). |
| **6** | `=`, `!`, `<`, `>` | Usa *lookahead* para distinguir asignación/operadores simples de relacionales dobles (`==`, `!=`, `<=`, `>=`). |
| **7** | `&`, `|` | Válidos en pares (`&&`, `||`). Si aparecen individuales → Error "Símbolo no reconocido". |
| **8** | Delimitadores y Operadores | Retorna token directo (`+`, `-`, `*`, `%`, `(`, `)`, `[`, `]`, `{`, `}`, `,`, `;`). |
| **9** | Cualquier otro carácter | Error "Símbolo no reconocido". Continúa el análisis sin detenerse. |

---

## 🏷️ Clasificación y Categorías de Tokens

| Categoría | Clase CSS | Tipos de Token Incluidos |
| --- | --- | --- |
| **Identificadores** | `.id-token` | `ID` (Muestra notación `<ID, attr>`) |
| **Números** | `.num-token` | `NUM_INT`, `NUM_DEC` |
| **Palabras Reservadas** | `.kw-token` | `KW_INT`, `KW_FLOAT`, `KW_CHAR`, `KW_BOOLEAN`, `KW_VOID`, `KW_IF`, `KW_ELSE`, `KW_FOR`, `KW_WHILE`, `KW_SCANF`, `KW_PRINTLN`, `KW_MAIN`, `KW_RETURN` |
| **Operadores** | `.op-token` | `ASSIGN` (`=`), `PLUS`, `MINUS`, `MULT`, `DIV`, `MOD`, `AND` (`&&`), `OR` (`||`), `NOT` (`!`), `COMP` |
| **Delimitadores** | `.sym-token` | `LPAREN`, `RPAREN`, `LBRACKET`, `RBRACKET`, `LBRACE`, `RBRACE`, `COMMA`, `SEMICOLON` |
| **Cadenas** | `.str-token` | `TEXTO` |

---

## 📄 Protocolo de Intercambio de Datos (JSON)

El motor de C++ devuelve la información estructurada a través de la siguiente especificación JSON:

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

## 🛠 Compilación y Ejecución

### Requisitos del Sistema

* **Compilador C++17** (`g++` / `gcc` v16+ o superior)
* **Librerías de desarrollo GTK+3 y WebKit2GTK**

En distribuciones basadas en Ubuntu/Debian/Arch Linux:

```bash
# Ubuntu / Debian
sudo apt update
sudo apt install build-essential libgtk-3-dev libwebkit2gtk-4.0-dev

# Arch Linux / Manjaro
sudo pacman -S base-devel gtk3 webkit2gtk

```

### Compilar el Proyecto

Para compilar el proyecto utilizando el `Makefile` incluido y generar el binario `lexlp`:

```bash
# Compilacion estándar
make

# Limpieza y re-compilacion completa
make clean && make

```

### Ejecutar la Aplicación

```bash
./lexlp

```

Al iniciar `lexlp`, la aplicación abre una ventana nativa WebView que realiza la encolación a través de `index.html` y presenta la interfaz gráfica principal (`lexer_ui.html`).

---

## 🧪 Pruebas Disponibles

Dentro del directorio `tests/` se encuentran los casos de prueba para evaluar los distintos escenarios del analizador:

* `prueba_basica.lp`: Declaración de variables simples y operaciones matemáticas.
* `prueba_completa.lp`: Cobertura de estructuras de control (`if/else`, `while`, `for`), arreglos y funciones I/O.
* `prueba_errores.lp`: Detección de símbolos inválidos (`@`, `$`), operadores lógicos incompletos (`&`, `|`) y constantes numéricas mal formadas (`1.2.3`).
* `prueba_limite.lp`: Manejo de casos borde como identificadores con guion bajo, comillas curvas tipográficas y números con punto al final.

---

## 📌 Limitaciones Conocidas

* **Comentarios Multilínea:** Solo se procesan comentarios de una sola línea (`//`).
* **Números que inician con punto:** Entradas como `.5` se clasifican como símbolo no reconocido (`.`) seguido del entero (`5`).
* **Conteo de Columnas en Cadenas UTF-8:** El cálculo de columnas cuenta los desplazamientos en bytes, por lo que caracteres multibyte dentro de una cadena pueden incrementar el contador de columna en más de 1 por carácter.

```

```