# Actualización — Analizador Sintáctico LP (Entregable 1)

**Fecha:** 2026-11-12 21:01:11 -05
**Modelo:** DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`)
**Objetivo:** documentar la implementación del Entregable 1 del proyecto de Compiladores.

> **Alcance de esta entrega:** integración con el analizador léxico e implementación inicial del analizador sintáctico, incluyendo estructura básica del programa, declaraciones y asignaciones.

---

## 1. Resumen ejecutivo

Se implementó un analizador sintáctico por **descenso recursivo** para el lenguaje LP. El parser consume la secuencia de tokens producida por el lexer existente, sin volver a analizar caracteres del archivo fuente.

La implementación cubre:

- `PROGRAMA`, `LISTA_FUNCIONES` y `PRINCIPAL`.
- `BLOQUE`, `LISTA_VARIABLES` y `COMANDOS`.
- Declaraciones con tipos, dimensiones y listas de identificadores.
- Asignaciones con expresiones y llamadas a función.
- Funciones top-level y parámetros.
- Detección de errores sintácticos con línea, columna, token encontrado y token esperado.
- Integración con CLI, interfaz gráfica y reportes en `output/`.

También se agregó el token `func`, requerido por la gramática, y se corrigieron varios casos límite del lexer que afectaban la integración.

---

## 2. Requisitos de la entrega

Según el documento de especificaciones, el Entregable 1 (30%) debía demostrar:

1. Integración funcional entre lexer y parser.
2. Lectura de un archivo `.lp` y obtención de su secuencia de tokens.
3. Reconocimiento de `PROGRAMA` y `PRINCIPAL`.
4. Validación de `BLOQUE` con llaves de apertura y cierre.
5. Implementación de `TIPO`, `TIPO_BASE`, `DIMENSION`, `LISTA_VARIABLES` y `LISTA_ID`.
6. Implementación de `ASIGNACION` y `COMPLEMENTO`.
7. Detección de errores básicos: `;` faltante, paréntesis/llaves sin cerrar y tokens inesperados.
8. Evidencias: código, archivos `.lp`, ejecución con un caso válido y dos inválidos, README.

La fecha de entrega indicada en el documento era el **13 de noviembre**.

---

## 3. Arquitectura implementada

```text
Archivo .lp
    │
    ▼
Lexer::analyze()
    │  vector<Token> + errores léxicos + tabla de símbolos
    ▼
Parser::parse()
    │  ParseResult { executed, accepted, errors }
    ▼
main.cpp
    ├── JSON para la GUI
    ├── reportes en output/
    └── resultado y errores por CLI
```

### Archivos principales

| Archivo | Responsabilidad |
|---|---|
| `src/Lexer.cpp` / `.h` | Análisis léxico y emisión de tokens |
| `src/Parser.cpp` / `.h` | Análisis sintáctico por descenso recursivo |
| `src/Token.h` | Tipos de token y serialización JSON |
| `src/SymbolTable.cpp` / `.h` | Tabla de símbolos |
| `src/main.cpp` | Integración lexer/parser, CLI, GUI y reportes |
| `ui/index.html` | Editor, pestañas y panel sintáctico |
| `ui/app.js` | IPC, renderizado y nueva pestaña |
| `ui/style.css` | Tema visual |
| `tests/sintactico/` | Casos válidos e inválidos del parser |
| `Makefile` | Compilación, dependencias y `make test` |

---

## 4. Cambios en el lexer

### 4.1 Token `func`

La gramática utiliza el terminal `func` para llamadas a función. El lexer original no lo reconocía.

Se agregó:

- `TokenType::KW_FUNC` en `src/Token.h`.
- La palabra reservada `func` en el mapa de `Lexer::check_keyword`.
- La representación `"FUNC"` en `tokenTypeToString`.
- La clasificación `FUNC` en la UI.

Ejemplo:

```lp
x = func suma(2, 3);
```

Secuencia de tokens relevante:

```text
<ID, x> <=> <FUNC> <ID, suma> <(> <NUM_INT> <,> <NUM_INT> <)>
```

### 4.2 Correcciones de integración

Se corrigieron casos que afectaban directamente la calidad de los tokens y del análisis:

- **BOM UTF-8:** se descarta al inicio del archivo; antes podía absorber la primera palabra.
- **Columnas con UTF-8:** dentro de cadenas se cuenta un carácter visible por codepoint, no un byte.
- **`\` + salto de línea:** se reporta la cadena como no cerrada y se conserva correctamente el conteo de líneas.
- **Escritura de tokens:** `output/tokens.txt` ya no termina con un espacio innecesario.

### 4.3 Mapeo gramática → tokens

| Terminal de la gramática | Token del lexer |
|---|---|
| `id` | `ID` |
| `num_int` | `NUM_INT` |
| `num_dec` | `NUM_DEC` |
| `texto` | `TEXTO` |
| `comp` | `COMP` |
| `func` | `FUNC` |
| `int`, `float`, `char`, `boolean` | `INT`, `FLOAT`, `CHAR`, `BOOLEAN` |
| `void` | `VOID` |
| `main` | `MAIN` |
| `if`, `else`, `while`, `return` | `IF`, `ELSE`, `WHILE`, `RETURN` |
| `scanf`, `println` | `SCANF`, `PRINTLN` |
| `=`, `+`, `-`, `*`, `/`, `%` | `=`, `+`, `-`, `*`, `/`, `%` |
| `&&`, `||`, `!` | `&&`, `||`, `!` |
| `(`, `)`, `[`, `]`, `{`, `}`, `,`, `;` | Los mismos símbolos |

El token `COMP` conserva el lexema exacto (`<`, `<=`, `>`, `>=`, `!=`, `==`) en `Token::lexeme`, aunque el parser lo trate como un único terminal `comp`.

---

## 5. Analizador sintáctico

### 5.1 Diseño

Se creó una clase `Parser` que recibe una referencia constante al vector de tokens:

```cpp
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    ParseResult parseProgram();
    static ParseResult parse(const std::vector<Token>& tokens);

private:
    const std::vector<Token>& tokens_;
    size_t pos_ = 0;
    std::vector<SyntaxError> errors_;
    Token eof_;
};
```

Características:

- **Descenso recursivo:** cada método privado corresponde a un no terminal.
- **Consumo de tokens:** no se accede al texto fuente.
- **EOF sintético:** si se llega al final, se genera un token `END_OF_FILE` con la última línea/columna.
- **Errores acumulables:** `ParseResult` contiene `executed`, `accepted` y un vector de `SyntaxError`.
- **Recuperación básica:** `synchronize()` avanza hasta `;`, `}` o el inicio de otro comando.
- **Sin análisis semántico:** no se comprueba declaración, tipos ni ejecución.

### 5.2 Correspondencia con la gramática

| Producción | Método |
|---|---|
| `PROGRAMA → LISTA_FUNCIONES PRINCIPAL` | `parseProgram()` |
| `LISTA_FUNCIONES → DECLARACION_FUNCION ... \| ε` | `parseListaFunciones()` |
| `DECLARACION_FUNCION → TIPO_RETORNO id ( PARAMETROS ) BLOQUE` | `parseDeclaracionFuncion()` |
| `TIPO_RETORNO → TIPO \| void` | `parseTipoRetorno()` |
| `TIPO → TIPO_BASE DIMENSION` | `parseTipo()` |
| `TIPO_BASE → char \| float \| int \| boolean` | `parseTipoBase()` |
| `DIMENSION → [ num_int ] DIMENSION \| ε` | `parseDimension()` |
| `PARAMETROS → TIPO id LISTA_PARAMETROS \| ε` | `parseParametros()` |
| `LISTA_PARAMETROS → , TIPO id ... \| ε` | `parseListaParametros()` |
| `PRINCIPAL → main ( ) BLOQUE` | `parsePrincipal()` |
| `BLOQUE → { LISTA_VARIABLES COMANDOS }` | `parseBloque()` |
| `LISTA_VARIABLES → TIPO id LISTA_ID ; ... \| ε` | `parseListaVariables()` |
| `LISTA_ID → , id LISTA_ID \| ε` | `parseListaId()` |
| `COMANDOS → COMANDO COMANDOS \| ε` | `parseComandos()` |
| `COMANDO → ...` | `parseComando()` |
| `ASIGNACION → id = COMPLEMENTO ;` | `parseAsignacion()` |
| `COMPLEMENTO → EXPRESION \| FUNCION` | `parseComplemento()` |
| `FUNCION → func id ( ARGUMENTOS )` | `parseFuncion()` |
| `ARGUMENTOS → EXPRESION NUEVO_ARGUMENTO \| ε` | `parseArgumentos()` |
| Expresiones `||`, `&&`, relacionales, aditivas y multiplicativas | `parseExpresionO()`, `parseExpresionY()`, `parseExpresionRelacional()`, `parseExpresionAditiva()`, `parseExpresionMultiplicativa()` |
| `FACTOR`, `TERMINO`, `DIMENSION2` | `parseFactor()`, `parseTermino()`, `parseDimension2()` |

### 5.3 Funcionalidades cubiertas

- Programa con `main()` y bloque.
- `void main()` y `main()`.
- Declaraciones simples y múltiples:
  - `int edad;`
  - `int a, b, c;`
  - `float promedio;`
  - `char letra;`
  - `boolean activo;`
- Arreglos:
  - `int datos[10];` (forma usada en los ejemplos).
  - `int[10] datos;` (forma literal de la gramática).
- Asignaciones:
  - `edad = 20;`
  - `promedio = 15.5;`
  - `total = (a + b) * 2 - 3;`
- Expresiones con precedencia:
  - `||`
  - `&&`
  - comparaciones
  - `+` y `-`
  - `*`, `/` y `%`
  - signos unarios, `!`, paréntesis, texto, IDs y constantes.
- Funciones top-level:
  - `int suma(int a, int b) { ... }`
  - `void main() { ... }`
- Llamadas con `func`:
  - `x = func suma(2, 3);`

### 5.4 Decisiones de compatibilidad

El documento de gramática y los ejemplos de la especificación tenían dos contradicciones:

1. La gramática define `PRINCIPAL → main ( ) BLOQUE`, pero todos los ejemplos usan `void main()`.
2. La gramática define `DIMENSION` como parte de `TIPO` (`int[10] datos`), pero los ejemplos usan la forma habitual `int datos[10]`.

Para que los ejemplos oficiales fueran aceptados, se decidió:

- Aceptar `void` opcional antes de `main`:
  ```text
  PRINCIPAL → [void] main ( ) BLOQUE
  ```
- Aceptar dimensiones en ambas posiciones:
  ```lp
  int[10] datos;   // forma de la gramática
  int datos[10];   // forma de los ejemplos
  ```

Ambas decisiones están documentadas en el README y deben mencionarse en la sustentación.

### 5.5 Manejo de errores

Cada error contiene:

```cpp
struct SyntaxError {
    int line;
    int col;
    std::string found;
    std::string expected;
    std::string message;
};
```

Ejemplo de salida:

```text
Línea 4, columna 5 (sintáctico): Se esperaba ';'; se encontró 'edad'.
```

El parser reporta:

- Punto y coma faltante.
- Llave de cierre faltante.
- Paréntesis de cierre faltante.
- Token inesperado al inicio de un comando.
- Declaración después de un comando.
- Expresiones incompletas.

Cuando el lexer produce errores, el parser **no se ejecuta** y el resultado se marca como `NO EJECUTADO` para evitar errores en cascada.

### 5.6 Limitaciones actuales

Todavía no forman parte del subconjunto implementado:

- `scanf` y `println`.
- `if`, `else` y `while`.
- `return` con expresión.
- AST.
- Recuperación avanzada para reportar múltiples errores sin cascadas.
- Análisis semántico.

`for` no pertenece a la gramática de LP. El lexer lo reconoce, pero el parser lo rechaza y lo informa.

---

## 6. Integración con CLI, GUI y reportes

### 6.1 Flujo en `src/main.cpp`

```cpp
LexerOutput output = Lexer::analyze(source_code, symTable);

ParseResult parseResult;
if (output.errors.empty()) {
    parseResult = Parser::parse(output.tokens);
} else {
    parseResult.executed = false;
    parseResult.accepted = false;
}
```

Luego se generan los reportes y se construye el JSON de respuesta.

### 6.2 JSON enviado a la GUI

El JSON del lexer se extiende con un objeto `syntax`:

```json
{
  "tokens": [],
  "symbols": [],
  "errors": [],
  "syntax": {
    "executed": true,
    "accepted": false,
    "errors": [
      {
        "line": 4,
        "col": 5,
        "found": "edad",
        "expected": "';'",
        "message": "Se esperaba ';'; se encontró 'edad'."
      }
    ]
  }
}
```

### 6.3 Reportes generados

| Archivo | Contenido |
|---|---|
| `output/tokens.txt` | Secuencia de tokens |
| `output/tabla_simbolos.txt` | Tabla de identificadores |
| `output/errores.txt` | Errores léxicos |
| `output/errores_sintacticos.txt` | Errores sintácticos |
| `output/resultado_sintactico.txt` | `ACEPTADO`, `RECHAZADO` o `NO EJECUTADO` |

El directorio `output/` se crea automáticamente y se comprueba que las escrituras se realicen correctamente.

### 6.4 Modo CLI

```bash
./lexlp tests/sintactico/valido_minimo.lp
```

Salida esperada:

```text
Resultado sintáctico: ACEPTADO
Resultados guardados en la carpeta output/
```

Códigos de salida:

| Código | Significado |
|---|---|
| `0` | Programa aceptado |
| `1` | Errores léxicos/sintácticos o de escritura |
| `2` | Uso incorrecto |

También se agregó:

```bash
./lexlp --help
```

### 6.5 Interfaz gráfica

Se agregó una pestaña **Sintáctico** con:

- Resultado global: aceptado, rechazado o no ejecutado.
- Contador de errores.
- Tabla con línea, columna, token encontrado, token esperado y mensaje.

El renderizado de errores y símbolos se cambió a `textContent`/`createElement` para evitar inyección de HTML con lexemas no confiables.

---

## 7. Pruebas implementadas

### 7.1 Casos válidos

| Archivo | Qué prueba |
|---|---|
| `tests/sintactico/valido_minimo.lp` | Ejemplo mínimo del enunciado |
| `tests/sintactico/valido_estricto.lp` | `main()` sin `void` y arreglos |
| `tests/sintactico/valido_expresiones.lp` | Expresiones con paréntesis y precedencia |
| `tests/sintactico/valido_funciones.lp` | Función top-level y llamada con `func` |

### 7.2 Casos inválidos

| Archivo | Error esperado |
|---|---|
| `tests/sintactico/invalido_sin_punto_coma.lp` | Falta de `;` |
| `tests/sintactico/invalido_llave_sin_cerrar.lp` | Falta de `}` |
| `tests/sintactico/invalido_parentesis_sin_cerrar.lp` | Falta de `)` |
| `tests/sintactico/invalido_token_inesperado.lp` | Token inesperado |
| `tests/sintactico/invalido_declaracion_despues_comando.lp` | Declaración después de un comando |

### 7.3 Ejecución

```bash
make test
```

Resultado final:

```text
Pruebas sintácticas completadas.
```

- 4 casos válidos aceptados.
- 5 casos inválidos rechazados.
- Código de salida `0`.

### 7.4 Pruebas históricas del lexer

Los archivos `prueba_basica.lp`, `prueba_completa.lp`, `prueba_errores.lp` y `prueba_limite.lp` siguen sirviendo para validar el lexer, pero no son todos válidos sintácticamente:

- `prueba_basica.lp` usa `println`, `return;` e inicialización en declaraciones.
- `prueba_completa.lp` usa `for` y `return;`.
- `prueba_errores.lp` y `prueba_limite.lp` contienen errores léxicos intencionales.

Por eso el parser puede rechazarlos. Los casos de aceptación sintáctica están en `tests/sintactico/`.

---

## 8. Compilación y ejecución

### Requisitos

- `g++` con C++17.
- `make`.
- `pkg-config`.
- GTK 3.
- WebKit2GTK 4.1.

### Comandos

```bash
# Compilar
make

# Recompilar desde cero
make clean && make

# Ejecutar pruebas sintácticas
make test

# GUI
./lexlp

# CLI
./lexlp tests/sintactico/valido_minimo.lp

# Ayuda
./lexlp --help
```

### Makefile

Se actualizó con:

- `-Wall -Wextra`.
- `-MMD -MP` para dependencias de cabeceras.
- Target `test`.
- Limpieza de `src/*.o`, `src/*.d` y `lexlp`.

---

## 9. Archivos nuevos y modificados

### Nuevos

- `src/Parser.h`
- `src/Parser.cpp`
- `tests/sintactico/valido_minimo.lp`
- `tests/sintactico/valido_estricto.lp`
- `tests/sintactico/valido_expresiones.lp`
- `tests/sintactico/valido_funciones.lp`
- `tests/sintactico/invalido_sin_punto_coma.lp`
- `tests/sintactico/invalido_llave_sin_cerrar.lp`
- `tests/sintactico/invalido_parentesis_sin_cerrar.lp`
- `tests/sintactico/invalido_token_inesperado.lp`
- `tests/sintactico/invalido_declaracion_despues_comando.lp`

### Modificados

- `src/Token.h`
- `src/Lexer.cpp`
- `src/SymbolTable.h`
- `src/SymbolTable.cpp`
- `src/main.cpp`
- `Makefile`
- `.gitignore`
- `README.md`
- `ui/index.html`
- `ui/app.js`
- `ui/style.css`

### Documentación

- `AGENTS.md`
- `actualizacion.md` (este archivo)

---

## 10. Estado de la entrega

El subconjunto solicitado para el Entregable 1 queda implementado y verificado:

- [x] Integración con el lexer.
- [x] Lectura de `.lp`.
- [x] `PROGRAMA` y `PRINCIPAL`.
- [x] `BLOQUE`.
- [x] Declaraciones y dimensiones.
- [x] Asignaciones y expresiones.
- [x] Errores sintácticos básicos.
- [x] Casos válidos e inválidos.
- [x] README actualizado.
- [x] Evidencia de ejecución.

### Pendientes para el Entregable 2

- Implementar `scanf` y `println`.
- Implementar `if`, `else` y `while`.
- Implementar `return` con expresión.
- Completar funciones y llamadas según la gramática.
- Construir el AST.
- Mejorar la recuperación de errores para reportar múltiples fallos.
- Integrar la visualización del AST en la GUI.

---

## 11. Notas

- El GIF `ui/carlos-gif.gif` y el rediseño de colores de la UI son cambios complementarios de interfaz; no forman parte del alcance funcional del parser.
- El repositorio no tiene commit realizado por esta implementación; los cambios quedan en el árbol de trabajo para revisión.
- Para la sustentación, conviene explicar el flujo `lexer → tokens → parser → ParseResult → reportes/JSON` y las dos decisiones de compatibilidad (`void main` y dimensiones prefijas/postfijas).

---

**Última actualización:** 2026-11-12 21:01:11 -05
