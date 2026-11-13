# LexLP Studio — Analizador Léxico y Sintáctico (Lenguaje LP)

[![Lenguaje](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![GUI](https://img.shields.io/badge/GUI-GTK%203%20%2B%20WebKit2GTK-green.svg)](https://www.gtk.org/)
[![Frontend](https://img.shields.io/badge/Frontend-HTML5%20%2F%20CSS3%20%2F%20JS-orange.svg)](https://developer.mozilla.org/)


**LexLP Studio** es el analizador léxico y sintáctico (Fases 1 y 2 del compilador) para el lenguaje de programación personalizado **LP**.

El proyecto implementa una arquitectura híbrida de alto desempeño: un **backend en C++17** que procesa el flujo léxico, gestiona la tabla de símbolos y detecta errores en una sola pasada, conectado mediante un **puente IPC nativo (GTK 3 + WebKit2GTK)** a una **interfaz gráfica moderna (UI)** desarrollada con tecnologías web. Además, cuenta con un **modo CLI** optimizado para evaluación y pruebas automatizadas por terminal.

---

## 🚀 Características Principales

* **Motor Léxico Integral (C++17):**
  * Reconocimiento según la regla del prefijo más largo (*Maximal Munch*) y autómatas finitos deterministas (DFA).
  * Soporte de *lookahead* para resolver operadores de 1 y 2 caracteres (`=`, `==`, `!`, `!=`, `<`, `<=`, `>`, `>=`, `&&`, `||`, `/`, `//`).
  * Normalización automática de comillas tipográficas UTF-8 (`“ ”`) a comillas dobles estándar (`"`).
  * Filtrado y descarte transparente de comentarios de una línea (`//.*\n`) actualizando filas y columnas.
* **Manejo Robusto de Errores Léxicos (Sin Falsos Positivos):**
  * **Números mal formados consolidados:** Cadenas inválidas como `1.2.3`, `1..2`, `42.` o `123abc` son absorbidas como un único error léxico en lugar de dividirse erróneamente en tokens parciales.
  * **Identificadores inválidos agrupados:** Variables ilegales como `@variableInvalida` o `$precio` se marcan completas como error léxico, evitando contaminar la tabla de símbolos o emitir tokens `<ID>` falsos.
  * **Cadenas no cerradas:** Detección de constantes de texto que no cierran comillas antes del salto de línea o fin de archivo.
* **Tabla de Símbolos en Tiempo Real:**
  * Almacenamiento en estructura híbrida (`std::unordered_map` y `std::vector`) para inserción y búsqueda en $O(1)$ garantizando orden secuencial sin identificadores duplicados.
  * Asociación de atributos posicionales en el formato `<ID, pos>`.
* **Doble Modo de Ejecución:**
  * **Modo GUI (Escritorio):** IDE interactivo con selector desplegable de pruebas, visor de tokens por categorías, tabla de símbolos y lista de errores en vivo.
  * **Modo CLI (Terminal):** Ejecución directa (`./lexlp archivo.lp`) diseñada para calificación automatizada.
* **Exportación Automática a Disco (`output/`):**
  * Genera tras cada análisis los archivos exigidos para la entrega: `tokens.txt`, `tabla_simbolos.txt` y `errores.txt`.

---

## 📁 Estructura del Proyecto

```text
.
├── Makefile                # Script de compilación (g++ C++17, GTK 3, WebKit2GTK)
├── README.md               # Documentación general del proyecto
├── lexlp                   # Binario ejecutable compilado
├── output/                 # Reportes generados en texto plano
│   ├── errores.txt         # Reporte tabulado de errores léxicos
│   ├── tabla_simbolos.txt  # Tabla de símbolos con identificadores únicos
│   └── tokens.txt          # Secuencia lineal continua de tokens
├── src/                    # Código fuente del motor en C++
│   ├── Lexer.h / .cpp      # Escáner léxico, DFA, lookahead y reglas
│   ├── Parser.h / .cpp     # Parser por descenso recursivo (Entregable 1)
│   ├── SymbolTable.h / .cpp# Tabla de símbolos (hash map + vector)
│   ├── Token.h / .cpp      # Estructuras Token, LexerError y TokenType
│   └── main.cpp            # Punto de entrada, modo CLI y ventana GTK/WebKit
├── tests/                  # Suites de prueba en lenguaje LP
│   ├── prueba_basica.lp    # Aritmética, variables y salida básica
│   ├── prueba_completa.lp  # Cobertura integral de tipos, control y operadores
│   ├── prueba_errores.lp   # Símbolos inválidos (@, $), 1.2.3, cadenas abiertas
│   ├── prueba_limite.lp    # Casos límite (guiones bajos, números mal formados)
│   └── sintactico/         # Casos válidos e inválidos del parser (Entregable 1)
└── ui/                     # Frontend de la interfaz gráfica
    ├── app.js              # Lógica de cliente, puente IPC y renderizado dinámico
    ├── carlos-gif.gif      # GIF decorativo siempre visible en la esquina
    ├── index.html          # Vista HTML del editor y paneles de resultados
    └── style.css           # Estilos visuales con tema oscuro tipo IDE
```

---

## 📜 Especificación de Tokens y Expresiones Regulares

| Expresión Regular | Token Generado | Formato de Salida | Categoría |
| :--- | :---: | :---: | :--- |
| `D = [0-9]` *(Auxiliar)* | — | — | Dígito numérico |
| `L = [a-zA-Z_]` *(Auxiliar)* | — | — | Carácter de identificador |
| `NUM_INT = D+` | `NUM_INT` | `<NUM_INT>` | Número entero |
| `NUM_DEC = D+\.D+` | `NUM_DEC` | `<NUM_DEC>` | Número decimal |
| `ID = L(L\|D)*` | `ID` | `<ID, pos>` | Identificador *(con posición en tabla)* |
| `TEXTO = “.*”` o `".*"` | `TEXTO` | `<TEXTO>` | Constante de texto |
| `int`, `float`, `char`, `boolean`, `void` | `INT`, `FLOAT`, etc. | `<TIPO>` | Palabras reservadas (Tipos) |
| `if`, `else`, `for`, `while` | `IF`, `ELSE`, etc. | `<PALABRA>` | Palabras reservadas (Control) |
| `scanf`, `println`, `main`, `return`, `func` | `SCANF`, `PRINTLN`, etc. | `<PALABRA>` | Palabras reservadas (E/S, función y llamadas) |
| `COMENT = //.*\n` | *(Ignorado)* | — | Comentario de una línea *(sin token)* |
| `=` | `=` | `<=>` | Operador de asignación |
| `+`, `-`, `*`, `/`, `%` | `+`, `-`, `*`, `/`, `%` | `<+>`, `<->`, etc. | Operadores aritméticos |
| `&&`, `\|\|`, `!` | `&&`, `\|\|`, `!` | `<&&>`, `<\|\|>`, `<!>` | Operadores lógicos |
| `COMP = > \| >= \| < \| <= \| != \| ==` | `COMP` | `<COMP>` | Operadores de comparación / relacionales |
| `(`, `)`, `[`, `]`, `{`, `}`, `,`, `;` | `(`, `)`, etc. | `<(>`, `<;>`, etc. | Símbolos especiales / delimitadores |

---

## 🧩 Analizador Sintáctico (Entregable 1)

El parser se implementa mediante **descenso recursivo** y consume directamente la
secuencia de tokens producida por `Lexer::analyze()`. No vuelve a analizar
caracteres del archivo fuente.

### Alcance actual

- `PROGRAMA → LISTA_FUNCIONES PRINCIPAL`
- `PRINCIPAL → [void] main ( ) BLOQUE`
- `BLOQUE → { LISTA_VARIABLES COMANDOS }`
- Declaraciones: `TIPO`, `TIPO_BASE`, `DIMENSION`, `LISTA_VARIABLES` y `LISTA_ID`.
- Asignaciones: `ASIGNACION`, `COMPLEMENTO` y la jerarquía de expresiones.
- Funciones top-level básicas y llamadas con `func` como complemento de asignación.
- Detección de errores con línea, columna, token encontrado y token esperado.

Se acepta tanto `void main()` (forma usada en los ejemplos del enunciado) como
`main()` (forma literal de la gramática). También se aceptan dimensiones
posteriores al identificador (`int datos[10];`, usada en los ejemplos) además de
la forma prefija de la gramática (`int[10] datos;`).

### Limitaciones de esta entrega

`scanf`, `println`, `if`, `else`, `while`, `return` y `for` todavía no forman
parte del subconjunto implementado. El parser los detecta y reporta un error
comprensible; se completarán en el Entregable 2. `for` no pertenece a la
gramática de LP.

### Resultado del parser

El análisis sintáctico se considera exitoso solo si el lexer no reportó errores
y no se encontró ningún error sintáctico. Los resultados se guardan en:

- `output/errores_sintacticos.txt`
- `output/resultado_sintactico.txt`

---

## ⚙️ Arquitectura y Flujo de Comunicación

```mermaid
flowchart TD
    subgraph UI ["Frontend (ui/)"]
        EDITOR["Editor de Código Fuente"]
        SELECTOR["Desplegable de Pruebas (.lp)"]
        TABS["Pestañas: Tokens | Símbolos | Errores | Sintáctico"]
        JS["app.js (Bridge IPC)"]
        
        SELECTOR -->|Carga prueba| EDITOR
        EDITOR -->|postMessage 'ipc'| JS
        JS -->|Renderiza DOM| TABS
    end

    subgraph Backend ["Backend C++ (src/)"]
        MAIN["main.cpp (Servidor GTK / WebKit / CLI)"]
        LEXER["Lexer::analyze()"]
        SYMTABLE["SymbolTable"]
        PARSER["Parser::parse()"]

        MAIN --> LEXER
        LEXER --> SYMTABLE
        LEXER --> PARSER
    end

    subgraph Output ["Archivos de Salida (output/)"]
        TOK["tokens.txt"]
        SYM["tabla_simbolos.txt"]
        ERR["errores.txt"]
    end

    JS -- "Código fuente" --> MAIN
    MAIN -- "Resultados (JSON + syntax)" --> JS
    PARSER -- "Errores sintácticos" --> MAIN
    MAIN --> TOK
    MAIN --> SYM
    MAIN --> ERR
```

---

## 🛠️ Instalación y Compilación

### Prerrequisitos (Linux)

El proyecto requiere un compilador con soporte para **C++17** y las librerías de desarrollo de **GTK+ 3** y **WebKit2GTK** (versión 4.1 o 4.0):

#### En Ubuntu / Debian / Linux Mint:
```bash
sudo apt update
sudo apt install build-essential pkg-config libgtk-3-dev libwebkit2gtk-4.1-dev
# Si tu distribución utiliza la rama 4.0:
# sudo apt install libwebkit2gtk-4.0-dev
```

#### En Arch Linux / Manjaro:
```bash
sudo pacman -S base-devel pkgconf gtk3 webkit2gtk-4.1
```

#### En Fedora / RHEL:
```bash
sudo dnf install gcc-c++ make pkgconfig gtk3-devel webkit2gtk4.1-devel
```

### Compilación

Para compilar el proyecto y generar el binario ejecutable `lexlp`:

```bash
# Compilación estándar
make

# Limpieza y recompilación completa desde cero
make clean && make

# Pruebas del subconjunto sintáctico del Entregable 1
make test
```

---

## 🚀 Uso del Programa

### 1. Modo Gráfico (GUI)
Ejecuta el binario sin argumentos:
```bash
./lexlp
```
1. En la barra superior, usa el **menú desplegable** para elegir una prueba. Las opciones “Sintaxis válida” y “Sintaxis inválida” están pensadas para demostrar el parser; las demás son pruebas del lexer.
2. Modifica o escribe código libremente en el panel izquierdo. El editor muestra la numeración de líneas a la izquierda y la posición actual del cursor en el encabezado (`L: línea | C: columna`).
3. Presiona **▶ Analizar Código**.
4. Revisa los resultados en las pestañas derechas:
   * **Tokens:** Etiquetas coloreadas con formato `<TIPO>` o `<ID, pos>`.
   * **Tabla de Símbolos:** Relación `Posición` e `Identificador`.
   * **Errores Léxicos:** Tabla con `Línea`, `Columna`, `Lexema` y estado `ERROR_LEXICO`.
   * **Sintáctico:** Resultado `ACEPTADO`/`RECHAZADO`, errores del parser y estado “no ejecutado” cuando el lexer falla.
5. En la esquina inferior derecha se muestra `ui/carlos-gif.gif` en bucle infinito, siempre por encima de los paneles y sin bloquear la interacción.

### 2. Modo Consola (CLI)
Ideal para pruebas automatizadas y corrección docente por lotes:
```bash
./lexlp tests/prueba_basica.lp
./lexlp tests/prueba_completa.lp
./lexlp tests/prueba_errores.lp
./lexlp tests/prueba_limite.lp

# Pruebas del parser (Entregable 1)
./lexlp tests/sintactico/valido_minimo.lp
./lexlp tests/sintactico/invalido_sin_punto_coma.lp
```
El analizador procesará el archivo fuente, ejecutará el lexer y el parser, e imprimirá el resultado sintáctico (`ACEPTADO` o `RECHAZADO`) junto con los errores encontrados. Las salidas se escriben en la carpeta `output/`.

Códigos de salida del modo CLI:

| Código | Significado |
|:---:|---|
| `0` | Programa aceptado sintácticamente |
| `1` | Errores léxicos o sintácticos, o error de lectura/escritura |
| `2` | Uso incorrecto del comando |

Los archivos del lexer se siguen generando aunque el parser rechace el programa, para conservar la evidencia de la fase léxica.

> Las pruebas históricas del lexer usan construcciones que todavía no forman parte del subconjunto sintáctico (`println`, `return;`, `for`), por lo que el parser puede rechazarlas. Para la aceptación sintáctica deben usarse los casos de `tests/sintactico/`.

---

## 🧪 Casos de Prueba Incluidos (`tests/`)

* **[`tests/prueba_basica.lp`](tests/prueba_basica.lp):** Declaración de variables `int` y `float`, operadores aritméticos básicos (`+`, `*`), cadenas de texto y llamada a `println`.
* **[`tests/prueba_completa.lp`](tests/prueba_completa.lp):** Cobertura exhaustiva de palabras clave, tipos (`char`, `boolean`), arreglos unidimensionales (`datos[10]`), bucles `while`/`for`, condicionales `if`/`else`, operadores lógicos compuestos (`&&`, `||`, `!`), comparaciones relacionales (`<=`, `>=`, `!=`) y comentarios.
* **[`tests/prueba_errores.lp`](tests/prueba_errores.lp):** Verificación de detección de símbolos no permitidos (`@variableInvalida`, `$precio`), operadores lógicos solitarios (`&`, `|`), números mal formados (`1.2.3`) y cadenas no cerradas.
* **[`tests/prueba_limite.lp`](tests/prueba_limite.lp):** Casos límite (*edge cases*): identificadores válidos con guiones bajos iniciales (`_contador`, `__init__`), decimales sin dígitos tras el punto (`42.`), múltiples puntos (`1..2`, `1.2.3.4`), números pegados a letras (`123abc`, `1.2x`) y cadenas con comillas escapadas.

### Pruebas del analizador sintáctico

La carpeta [`tests/sintactico/`](tests/sintactico/) contiene los casos del Entregable 1:

* `valido_minimo.lp`: ejemplo mínimo del enunciado con `void main()`.
* `valido_estricto.lp`: `main()` sin `void` y declaraciones con arreglos.
* `valido_expresiones.lp`: asignaciones con expresiones y paréntesis.
* `valido_funciones.lp`: función top-level y llamada con `func`.
* `invalido_sin_punto_coma.lp`: falta de `;` en una declaración.
* `invalido_llave_sin_cerrar.lp`: bloque sin `}`.
* `invalido_token_inesperado.lp`: token que no inicia un comando.
* `invalido_declaracion_despues_comando.lp`: declaración después de un comando.
* `invalido_parentesis_sin_cerrar.lp`: expresión sin `)`.

---

## 📄 Archivos de Salida Generados (`output/`)

Tras cada ejecución (tanto en GUI como en CLI), se actualizan automáticamente:

1. **`output/tokens.txt`:** Cadena continua de tokens delimitados por espacios:
   ```text
   <VOID> <MAIN> <(> <)> <{> <INT> <ID,0> <=> <NUM_INT> <;> <RETURN> <;> <}>
   ```
2. **`output/tabla_simbolos.txt`:** Lista tabulada de identificadores únicos registrados:
   ```text
   Posición	Identificador
   0		valorA
   1		valorB
   ```
3. **`output/errores.txt`:** Reporte tabulado de fallos léxicos detectados:
   ```text
   Línea	Columna	Lexema	Resultado
   5	9	@variableInvalida	ERROR_LEXICO
   9	28	1.2.3	ERROR_LEXICO
   ```
4. **`output/errores_sintacticos.txt`:** Errores del parser con línea, columna,
   token encontrado, token esperado y mensaje:
   ```text
   Línea	Columna	Encontrado	Esperado	Mensaje
   5	9	edad	;	Se esperaba ';'; se encontró 'edad'.
   ```
5. **`output/resultado_sintactico.txt`:** Resultado global (`ACEPTADO`,
   `RECHAZADO` o `NO EJECUTADO`).