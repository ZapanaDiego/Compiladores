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

# Analizador Léxico (Lexer)

Analizador léxico escrito en C++ para un mini-lenguaje de estilo C/Java. Recibe el código fuente como texto y lo convierte en una lista de **tokens**, detectando además los **errores léxicos** y construyendo una **tabla de símbolos** con los identificadores.

La salida completa se puede serializar a **JSON**, lista para consumirse desde una interfaz web o una API.

---

## Estructura del proyecto

| Archivo | Responsabilidad |
|---|---|
| `Lexer.h` | Declara la clase `Lexer` (interfaz pública: `analyze`, y la función auxiliar `check_keyword`) |
| `Lexer.cpp` | Implementación del análisis: recorre el texto y genera tokens y errores |
| `Token.h` | Define `TokenType`, `Token`, `LexerError` y `LexerOutput`, junto con su conversión a JSON |
| `SymbolTable.h` | Tabla de símbolos donde se registran los identificadores |

---

## El proceso de análisis

```
Código fuente (string)
        │
        ▼
  Lexer::analyze()
        │   recorre carácter por carácter
        ▼
  ┌───────────────┬────────────────┬──────────────────┐
  │    Tokens     │    Errores     │ Tabla de símbolos│
  └───────────────┴────────────────┴──────────────────┘
        │
        ▼
   LexerOutput  ──►  to_json()
```

### Función principal

```cpp
LexerOutput Lexer::analyze(const std::string& input, SymbolTable& symTable);
```

- **`input`**: el código fuente completo.
- **`symTable`**: tabla de símbolos donde se insertan los identificadores encontrados.
- **Retorna** un `LexerOutput` con los tokens, los errores y la tabla de símbolos en JSON.

### Estado interno

| Variable | Uso |
|---|---|
| `i` | Posición actual en el texto |
| `line` | Línea actual (empieza en 1) |
| `col` | Columna actual (empieza en 1) |
| `start_col` | Columna donde **empezó** el token actual (es la que se reporta) |

### Orden de reconocimiento

| Paso | Detecta | Resultado |
|---|---|---|
| 1 | `\n`, espacios, tabs, `\r` | Se ignoran (`\n` incrementa `line` y reinicia `col`) |
| 2 | Letra o `_` | Lee la palabra completa. Palabra reservada → `KW_*`; si no → `ID` y se inserta en la tabla de símbolos |
| 3 | Comilla `"` o comillas curvas `“ ”` | Lee la cadena hasta cerrarla → `TEXTO`; si no cierra en la línea → error |
| 4 | Dígito | Lee el número completo → `NUM_INT`, `NUM_DEC` o error de número mal formado |
| 5 | `/` | Si sigue otro `/` es un comentario (se ignora la línea); si no, es `DIV` |
| 6 | `=`, `!`, `<`, `>` | Usa *lookahead* para distinguir `=` de `==`, `!` de `!=`, etc. |
| 7 | `&`, `\|` | Solo son válidos dobles (`&&`, `\|\|`); solos generan error |
| 8 | `+ - * %` y `( ) [ ] { } , ;` | Token directo |
| 9 | Cualquier otro carácter | Error "Simbolo no reconocido" |

El lexer **no se detiene en el primer error**: registra el error, avanza y continúa, de modo que se reportan todos los problemas en una sola pasada.

---

## Tokens reconocidos

| Categoría | Tipos de token |
|---|---|
| Literales e identificadores | `NUM_INT`, `NUM_DEC`, `ID`, `TEXTO` |
| Palabras reservadas | `KW_INT`, `KW_FLOAT`, `KW_CHAR`, `KW_BOOLEAN`, `KW_VOID`, `KW_IF`, `KW_ELSE`, `KW_FOR`, `KW_WHILE`, `KW_SCANF`, `KW_PRINTLN`, `KW_MAIN`, `KW_RETURN` |
| Asignación | `ASSIGN` (`=`) |
| Aritméticos | `PLUS`, `MINUS`, `MULT`, `DIV`, `MOD` |
| Lógicos | `AND` (`&&`), `OR` (`\|\|`), `NOT` (`!`) |
| Relacionales | `COMP` (`==`, `!=`, `<`, `<=`, `>`, `>=`) |
| Delimitadores | `LPAREN`, `RPAREN`, `LBRACKET`, `RBRACKET`, `LBRACE`, `RBRACE`, `COMMA`, `SEMICOLON` |
| Especiales | `UNKNOWN`, `END_OF_FILE` |

> Todos los operadores de comparación comparten el tipo `COMP`; para distinguirlos hay que revisar el `lexeme`.

### Expresiones regulares equivalentes

| Token | Patrón |
|---|---|
| Identificador | `[a-zA-Z_][a-zA-Z0-9_]*` |
| `NUM_INT` | `[0-9]+` |
| `NUM_DEC` | `[0-9]+\.[0-9]+` |
| `TEXTO` | `"([^"\\\n]\|\\.)*"` |
| Comentario | `//.*` |

---

## Estructura de un token

```cpp
struct Token {
    TokenType type;      // categoría
    std::string lexeme;  // texto exacto leído
    int attribute = -1;  // índice en la tabla de símbolos (solo para ID)
    int line;            // línea de inicio
    int col;             // columna de inicio
};
```

Notación clásica `<token, atributo>` con `toString()`:

- `x` → `<ID,0>`
- `+` → `<+>`
- `int` → `<INT>`

---

## Tabla de símbolos

Cada identificador se registra con `symTable.insertOrGet(ident)`:

- Si es nuevo, se agrega y se devuelve su índice.
- Si ya existía, se devuelve el índice existente.

Ese índice se guarda en el atributo del token `ID`, por lo que **todas las apariciones de una misma variable comparten el mismo atributo**.

---

## Errores léxicos

| Mensaje | Cuándo ocurre | Ejemplo |
|---|---|---|
| `Cadena de texto no cerrada` | La cadena llega a fin de línea sin cierre | `"hola` |
| `Numero mal formado` | Más de un punto, letras pegadas o punto final | `1.2.3`, `12abc`, `5.` |
| `Simbolo no reconocido` | Carácter fuera del lenguaje o `&` / `\|` solitarios | `@`, `#`, `&` |

Cada error guarda línea, columna, lexema y mensaje.

---

## Ejemplo

**Entrada**

```c
int x = 3.5; // comentario
println("ok");
```

**Tokens generados**

| Token | Lexema | Línea:Col |
|---|---|---|
| `KW_INT` | `int` | 1:1 |
| `ID` | `x` | 1:5 |
| `ASSIGN` | `=` | 1:7 |
| `NUM_DEC` | `3.5` | 1:9 |
| `SEMICOLON` | `;` | 1:12 |
| `KW_PRINTLN` | `println` | 2:1 |
| `LPAREN` | `(` | 2:8 |
| `TEXTO` | `"ok"` | 2:9 |
| `RPAREN` | `)` | 2:13 |
| `SEMICOLON` | `;` | 2:14 |

El comentario se descarta.

---

## Uso

```cpp
#include "Lexer.h"
#include "SymbolTable.h"
#include <iostream>

int main() {
    std::string codigo = "int x = 3.5;";

    SymbolTable tabla;
    Lexer lexer;
    LexerOutput salida = lexer.analyze(codigo, tabla);

    std::cout << salida.to_json() << std::endl;
    return 0;
}
```

> Ajusta la construcción de `Lexer` según cómo esté declarada en tu `Lexer.h` (por ejemplo, si `analyze` es estático se llamaría como `Lexer::analyze(...)`).

## Formato de salida JSON

```json
{
  "tokens": [
    {"type":"INT","lexeme":"int","attr":null,"line":1,"col":1},
    {"type":"ID","lexeme":"x","attr":0,"line":1,"col":5}
  ],
  "errors": [
    {"line":3,"col":7,"lexeme":"@","message":"Simbolo no reconocido"}
  ],
  "symbols": [ ]
}
```

---

## Decisiones de diseño

- **Lookahead de un carácter** para operadores de uno o dos símbolos (`=`/`==`, `/`/`//`, etc.).
- **Lectura voraz de números**: se consume todo lo que parezca parte del número y luego se valida, así se reporta un solo error por número mal formado.
- **Soporte de comillas curvas UTF-8** (`“ ”`), útiles cuando el código se copia desde Word o páginas web; se normalizan a `"`.
- **Recuperación de errores**: ante un error se avanza y se sigue analizando.

## Limitaciones conocidas

- No hay comentarios multilínea (`/* ... */`).
- Números que empiezan con punto (`.5`) se reportan como símbolo no reconocido.
- Las columnas se cuentan por byte, por lo que caracteres UTF-8 multibyte dentro de cadenas suman más de una columna.
- Todos los operadores relacionales comparten el tipo `COMP`.


---

# Interfaz Gráfica (Frontend)

Además del lexer en C++, el proyecto incluye una interfaz web que se ejecuta dentro de un **WebView** (`window.webkit.messageHandlers`). El usuario escribe código en un editor, lo envía al lexer en C++ y visualiza los resultados en tres módulos: **tokens**, **tabla de símbolos** y **errores léxicos**.

## Arquitectura general

```
┌────────────────────────┐        código (string)        ┌────────────────────┐
│   Frontend (JS/HTML)   │ ────────────────────────────► │   Backend (C++)    │
│                        │   postMessage (IPC)           │                    │
│  - Editor              │                               │  Lexer::analyze()  │
│  - Pestañas            │ ◄──────────────────────────── │  LexerOutput       │
│  - Renderizado         │   showResults(jsonStr)        │  .to_json()        │
└────────────────────────┘        JSON (string)          └────────────────────┘
```

El JSON que recibe el frontend es exactamente el que genera `LexerOutput::to_json()`:

```json
{
  "tokens":  [ {"type":"ID","lexeme":"x","attr":0,"line":1,"col":5} ],
  "errors":  [ {"line":3,"col":7,"lexeme":"@","message":"Simbolo no reconocido"} ],
  "symbols": [ {"pos":0,"id":"x"} ]
}
```

---

## Componentes del frontend

### 1. Bridge de comunicación con C++

```js
window.analyzeCode = function(code) {
  return new Promise((resolve) => {
    window.showResults = function(jsonStr) { resolve(jsonStr); };
    window.webkit.messageHandlers.ipc.postMessage(code);
  });
};
```

Convierte la comunicación asíncrona con C++ en una **Promesa**:

1. Define `window.showResults`, la función que C++ invocará al terminar.
2. Envía el código al backend con `postMessage` por el canal `ipc`.
3. Cuando C++ llama a `showResults(json)`, la promesa se resuelve con el JSON.

Esto permite usar `await window.analyzeCode(code)` como si fuera una llamada normal.

### 2. Manejo de pestañas

Cada botón `.tab-btn` tiene un atributo `data-tab` con el `id` del panel que debe mostrar. Al hacer clic:

1. Se quita la clase `active` de todos los botones y paneles.
2. Se activa el botón pulsado y su panel correspondiente.

### 3. Contador de línea y columna

```js
editor.addEventListener('keyup', updateCursorPos);
editor.addEventListener('click', updateCursorPos);
```

`updateCursorPos()` toma el texto desde el inicio hasta el cursor (`selectionStart`), lo separa por `\n` y calcula:

- **Línea** = número de fragmentos.
- **Columna** = longitud del último fragmento + 1.

Usa la misma convención que el lexer (línea y columna empiezan en 1), así la posición del cursor coincide con las posiciones reportadas en tokens y errores.

### 4. Colección de pruebas

El objeto `testSamples` contiene programas de ejemplo, cargables desde el desplegable `sampleSelect` mediante `loadSelectedSample()`:

| Clave | Propósito |
|---|---|
| `basic` | Variables y expresiones aritméticas |
| `complete` | Cobertura integral: arreglos, `while`, `for`, `if/else`, `scanf`, operadores lógicos y relacionales |
| `errors` | Errores léxicos: `@`, `$`, `&` y `\|` solitarios, `1.2.3`, cadena sin cerrar |
| `edge_cases` | Casos límite: identificadores con `_`, `42.`, `1..2`, `123abc`, `1.2x`, comillas escapadas |
| `standard` | Programa corto con un error léxico (`@`) para demostración rápida |

### 5. Ejecución del análisis

```js
async function executeAnalysis() {
  const code = editor.value;
  if (!code.trim()) return;

  const rawResponse = await window.analyzeCode(code);
  const data = JSON.parse(rawResponse);

  renderTokens(data.tokens || []);
  renderSymbolTable(data.symbols || []);
  renderErrors(data.errors || []);
}
```

Flujo: valida que el editor no esté vacío → envía el código a C++ → parsea el JSON → delega el dibujado en tres funciones independientes. El `|| []` protege contra campos ausentes, y todo va dentro de un `try/catch` que registra fallos de comunicación en consola.

---

## Módulos de visualización

### Módulo 1: Tokens (`renderTokens`)

Dibuja cada token como una etiqueta (`token-tag`) con formato de compilador:

- `<ID, 0>` para identificadores (incluyen su posición en la tabla de símbolos).
- `<INT>`, `<+>`, `<COMP>`... para los demás.

Cada etiqueta recibe una clase CSS según su categoría:

| Clase CSS | Categoría | Tipos que incluye |
|---|---|---|
| `id-token` | Identificadores | `ID` |
| `num-token` | Números | `NUM_INT`, `NUM_DEC` |
| `kw-token` | Palabras reservadas | `INT`, `FLOAT`, `CHAR`, `BOOLEAN`, `VOID`, `IF`, `ELSE`, `FOR`, `WHILE`, `SCANF`, `PRINTLN`, `MAIN`, `RETURN` |
| `op-token` | Operadores | `=`, `+`, `-`, `*`, `/`, `%`, `&&`, `\|\|`, `!`, `COMP` |
| `sym-token` | Delimitadores | `(`, `)`, `[`, `]`, `{`, `}`, `,`, `;` |
| `str-token` | Cadenas | `TEXTO` |

Al pasar el mouse sobre un token, el `title` muestra su lexema, línea y columna.

> Los nombres de tipo (`INT`, `+`, etc.) coinciden con lo que devuelve `tokenTypeToString` en `Token.h`. Si cambias esos nombres en C++, hay que actualizar también las listas del frontend.

### Módulo 2: Tabla de símbolos (`renderSymbolTable`)

Tabla de dos columnas:

| Columna | Campo JSON | Contenido |
|---|---|---|
| Posición | `pos` | Índice del identificador (el mismo `attr` de los tokens `ID`) |
| Identificador | `id` | Nombre del identificador |

Si no hay identificadores se muestra "No hay identificadores."

### Módulo 3: Errores léxicos (`renderErrors`)

| Columna | Campo JSON |
|---|---|
| Línea | `line` |
| Columna | `col` |
| Lexema | `lexeme` |
| Tipo | fijo: `ERROR_LEXICO` |

Si no hay errores se muestra "✓ Sin errores léxicos." Cada módulo también actualiza su contador (`tokenCount`, `symbolCount`, `errorCount`).

> El campo `message` del error (por ejemplo "Numero mal formado") llega en el JSON pero la tabla actual no lo muestra.

---

## Flujo completo de una ejecución

1. El usuario escribe código o elige una prueba del desplegable.
2. Se llama a `executeAnalysis()`.
3. El código viaja a C++ por `postMessage`.
4. `Lexer::analyze` genera tokens, errores y tabla de símbolos.
5. C++ serializa el `LexerOutput` a JSON y llama a `showResults(json)`.
6. El frontend parsea el JSON y actualiza las tres pestañas.

## Elementos HTML requeridos

El script espera que existan estos IDs y clases en el HTML:

| Elemento | Uso |
|---|---|
| `#codeEditor` | Área de texto del editor |
| `#lineColCounter` | Indicador `L: x \| C: y` |
| `#sampleSelect` | Desplegable de pruebas |
| `.tab-btn` (con `data-tab`) y `.tab-pane` | Sistema de pestañas |
| `#tokensContainer`, `#tokenCount` | Módulo de tokens |
| `#symbolTableBody`, `#symbolCount` | Módulo de símbolos |
| `#errorsTableBody`, `#errorCount` | Módulo de errores |

## Limitaciones conocidas del frontend

- **Sin escape de HTML**: lexemas como `<` o `&` se insertan con `innerHTML` en la tabla de errores, lo que puede deformar la vista. Convendría usar `textContent` o una función de escape.
- **Un solo análisis a la vez**: `showResults` se redefine en cada llamada, por lo que lanzar dos análisis simultáneos sobrescribiría el primero.
- **Dependencia del WebView**: `window.webkit.messageHandlers` solo existe dentro de un WebView de WebKit; en un navegador normal `analyzeCode` fallará.
- **Contador de columna**: `keyup` y `click` no cubren todos los casos (por ejemplo, selección con teclas mantenidas o pegado con el mouse); se podría añadir el evento `input` o `selectionchange`.


