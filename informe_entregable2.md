# Informe técnico — Entregable 2

**Proyecto:** LexLP Studio — Analizador sintáctico para LP  
**Curso:** Compiladores  
**Entrega:** 24 de noviembre de 2026

## 1. Arquitectura

```text
Archivo .lp / editor
        │
        ▼
Lexer::analyze() ──► tokens + tabla de símbolos + errores léxicos
        │
        ▼
Parser::parse() ──► AST + errores sintácticos
        │
        ├── output/tokens.txt
        ├── output/tabla_simbolos.txt
        ├── output/errores.txt
        ├── output/errores_sintacticos.txt
        ├── output/resultado_sintactico.txt
        ├── output/ast.txt
        └── output/ast.json
        │
        ▼
JSON ──► showResults() ──► pestañas: Tokens, Símbolos, Errores, Sintáctico, AST
```

El parser consume la secuencia de tokens producida por el lexer. No vuelve a
analizar caracteres del código fuente. El análisis semántico queda fuera del
alcance: no se comprueba declaración de variables, compatibilidad de tipos ni
ejecución.

## 2. Método de análisis

Se utiliza **descenso recursivo predictivo** (top-down, LL(1) en la gramática
base). Cada no terminal tiene un método en `Parser`:

| Producción | Método |
|---|---|
| `PROGRAMA → LISTA_FUNCIONES PRINCIPAL` | `parseProgram()` |
| `LISTA_FUNCIONES → DECLARACION_FUNCION LISTA_FUNCIONES \| ε` | `parseListaFunciones()` |
| `DECLARACION_FUNCION → TIPO_RETORNO id ( PARAMETROS ) BLOQUE` | `parseDeclaracionFuncion()` |
| `TIPO_RETORNO → TIPO \| void` | `parseTipoRetorno()` |
| `TIPO → TIPO_BASE DIMENSION` | `parseTipo()` |
| `DIMENSION → [ num_int ] DIMENSION \| ε` | `parseDimension()` |
| `PARAMETROS → TIPO id LISTA_PARAMETROS \| ε` | `parseParametros()` |
| `PRINCIPAL → [void] main ( ) BLOQUE` | `parsePrincipal()` |
| `BLOQUE → { LISTA_VARIABLES COMANDOS }` | `parseBloque()` |
| `LISTA_VARIABLES → TIPO id LISTA_ID ; ... \| ε` | `parseListaVariables()` |
| `LISTA_ID → , id LISTA_ID \| ε` | `parseListaId()` |
| `COMANDOS → COMANDO COMANDOS \| ε` | `parseComandos()` |
| `COMANDO → ...` | `parseComando()` |
| `LECTURA → scanf ( ... ) ;` | `parseLectura()` |
| `ESCRITURA → println ( ... ) ;` | `parseEscritura()` |
| `SELECCION → if ( EXPRESION ) BLOQUE SINO` | `parseSeleccion()` |
| `MIENTRAS → while ( EXPRESION ) BLOQUE` | `parseMientras()` |
| `RETORNO → return EXPRESION ;` | `parseRetorno()` |
| `ASIGNACION → id = COMPLEMENTO ;` | `parseAsignacion()` |
| `COMPLEMENTO → EXPRESION \| FUNCION` | `parseComplemento()` |
| `FUNCION → func id ( ARGUMENTOS )` | `parseFuncion()` |
| Expresiones y precedencia | `parseExpresion*()`, `parseFactor()`, `parseTermino()` |
| `DIMENSION2 → [ EXPRESION_ADITIVA ] ...` | `parseDimension2()` |

## 3. AST

El AST se implementa en `src/AST.h` y `src/AST.cpp` con una estructura genérica
de nodos:

```cpp
struct ASTNode {
    ASTKind kind;
    std::string value;
    std::string valueType;
    Token token;
    std::vector<ASTChild> children;
};
```

Cada hijo tiene un rol (`condition`, `then`, `else`, `body`, `term`,
`argument`, `left`, `right`, etc.) para que el árbol sea autoexplicativo.

Tipos de nodo principales:

- Programa: `Program`, `Function`, `Main`, `Block`.
- Declaraciones: `VarDecl`, `Declarator`, `Type`, `Dimension`, `Param`.
- Comandos: `Assign`, `Read`, `Write`, `If`, `While`, `Return`, `Call`.
- Expresiones: `Binary`, `Unary`, `Literal`, `Identifier`, `Index`.

El AST se serializa en:

- `output/ast.txt`: árbol textual con indentación y roles.
- `output/ast.json`: estructura para la pestaña AST de la GUI.

Solo se genera cuando el programa es aceptado sintácticamente. Si hay errores,
la UI muestra “AST no disponible” y el JSON contiene `null`.

## 4. Manejo de errores

Cada `SyntaxError` incluye línea, columna, token encontrado, token esperado y
mensaje. El parser:

- Acumula errores y evita duplicados exactos en la misma posición.
- Se recupera con `recoverCommand()`, `recoverTopLevel()` y `synchronize()`.
- No consume el `}` que cierra el bloque actual al recuperarse.
- Distingue errores específicos: `else` sin `if`, `for` fuera de LP,
  `int main()`, `return;`, `scanf()`/`println()` vacíos.
- Se detiene si hay errores léxicos para evitar cascadas.

## 5. Pruebas

- `make test`: ejecuta los casos válidos e inválidos de `tests/sintactico/` y
  `tests/e2/`.
- `make test-ast`: verifica `output/ast.txt`, `output/ast.json` y que el JSON sea
  válido.
- Pruebas válidas del E2: integral, funciones, E/S, arreglos, control,
  expresiones, `main()` sin `void` y llamada `func` como comando.
- Pruebas inválidas del E2: punto y coma, llaves, paréntesis, `else` sin `if`,
  `scanf`/`println` vacíos, `return;`, `for`, `int main()`, declaración después
  de comando y llamada sin cierre.

## 6. Decisiones y limitaciones

- Se mantiene `void main()` como compatibilidad con los ejemplos, además de
  `main()`.
- `int main()` se rechaza con un mensaje específico.
- `return;` se rechaza porque la gramática exige una expresión.
- `for` no pertenece a LP.
- Se aceptan dimensiones postfijas (`int datos[10]`) y asignaciones a arreglos
  (`datos[i] = ...`) como compatibilidad con los ejemplos.
- Se acepta `;` opcional después de una llamada `func` como comando.
- La asociatividad del AST para operadores binarios es por la izquierda, aunque
  la gramática esté escrita con recursión por la derecha; el reconocimiento
  sintáctico es equivalente y se documenta esta decisión.
- No hay análisis semántico ni ejecución.
- Las rutas de `ui/` y `output/` siguen siendo relativas al directorio de
  trabajo.

## 7. Alternativas estudiadas

Se comparó el descenso recursivo con:

- **LL(1) con tabla:** misma clase de gramática, pero requiere FIRST/FOLLOW,
  pila explícita y una pila semántica para el AST; menos natural para mensajes
  de error personalizados.
- **LR/LALR con Bison:** más potente, pero genera código y dificulta el control
  de errores; además no cumple el requisito de métodos asociados a no
  terminales.
- **ANTLR/JavaCC:** generadores de parsers; agregan dependencias y código
  generado.
- **PEG/parser combinators:** expresivos, pero con backtracking y menor control.
- **Pratt/precedence climbing:** excelente para expresiones, pero no cubre toda
  la gramática.

Se eligió descenso recursivo porque la gramática es LL(1), la especificación lo
exige, facilita la construcción del AST y permite mensajes de error claros.

## 8. Distribución del trabajo

Completar con los nombres de los integrantes y los módulos asignados. Se
recomienda que todos conozcan el flujo completo: lexer, tokens, parser, AST y
reporte de errores.
