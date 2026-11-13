# AGENTS.md — LexLP Studio

**Última actualización:** 2026-11-12 21:02:13 -05
**Modelo:** DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`)
**Estado:** Activo

> **Regla principal:** este archivo debe actualizarse **cada vez que se lea** y **cada vez que se modifique cualquier archivo del proyecto**. Toda actualización debe quedar registrada con fecha, hora, zona horaria, modelo y detalle de lo realizado.

---

## 1. Reglas obligatorias para agentes

1. **Leer este archivo antes de trabajar** en el repositorio.
2. **Actualizarlo en cada lectura.** Aunque no se haya modificado código, agregar una entrada en el registro de actividad indicando “Lectura/verificación sin cambios”.
3. **Actualizarlo ante cualquier modificación.** Si se crea, edita, elimina, renombra o mueve un archivo, debe quedar registrado aquí.
4. **No eliminar entradas anteriores.** El registro es cronológico y acumulativo. Si algo se corrige, se agrega una nueva entrada que lo indique.
5. **Registrar siempre:**
   - fecha (`YYYY-MM-DD`),
   - hora (`HH:MM:SS`),
   - zona horaria,
   - modelo exacto utilizado,
   - acción realizada,
   - archivos afectados.
6. **Mantener actualizados** los apartados “Estado actual”, “Problemas conocidos” y “Backlog priorizado” cuando cambie la realidad del proyecto.
7. **No inventar resultados.** Toda afirmación sobre compilación, pruebas o comportamiento debe haber sido verificada o marcarse explícitamente como pendiente.
8. **Respetar el trabajo del usuario.** No borrar ni sobrescribir cambios ajenos sin confirmación.
9. **Idioma del proyecto:** español para documentación, mensajes de interfaz y comentarios nuevos.
10. **No versionar artefactos generados** (`output/*.txt`, binarios, objetos) salvo que el usuario lo pida explícitamente.

---

## 2. Contexto del proyecto

**LexLP Studio** es el analizador léxico (fase 1 de un compilador) para el lenguaje **LP**.

Arquitectura:

- **Backend C++17:** escáner léxico, tabla de símbolos y reportes.
- **GUI:** GTK 3 + WebKit2GTK con frontend HTML/CSS/JS.
- **CLI:** ejecución directa para pruebas y evaluación.
- **Salidas:** `output/tokens.txt`, `output/tabla_simbolos.txt`, `output/errores.txt`.

Flujo principal:

```text
ui/index.html + ui/app.js
        │ postMessage("ipc")
        ▼
src/main.cpp ──► Lexer::analyze() ──► SymbolTable
        │                │
        │                ├── output/tokens.txt
        │                ├── output/tabla_simbolos.txt
        │                └── output/errores.txt
        ▼
Respuesta JSON ──► showResults() ──► pestañas de la UI
```

---

## 3. Estructura real del repositorio

| Ruta | Rol | Estado |
|---|---|---|
| `src/Lexer.cpp` / `.h` | Escáner léxico | Activo |
| `src/Token.h` | Tipos de token, error y salida JSON | Activo |
| `src/Token.cpp` | Solo incluye `Token.h` | Activo, sin lógica |
| `src/SymbolTable.cpp` / `.h` | Tabla de símbolos y exportación | Activo |
| `src/main.cpp` | Punto de entrada CLI + ventana GTK/WebKit | Activo |
| `ui/index.html` | UI vigente | Activo |
| `ui/app.js` | Lógica de UI, IPC y renderizado | Activo |
| `ui/style.css` | Estilos de la UI vigente | Activo |
| `tests/*.lp` | Casos de prueba oficiales | Activo, sin salidas esperadas |
| `output/` | Reportes generados | Generado; no debería versionarse |
| `Makefile` | Compilación C++17 + GTK + WebKit | Activo |
| `README.md` | Documentación general | Activo, con detalles desactualizados |
| `index.html` (raíz) | UI antigua que espera `window.analizarCodigo` | **Muerto** |
| `lexer_ui.html`, `lexer_ui.js`, `lexer_ui.css` | UI antigua duplicada | **Muerto** |
| `argo.toml` | Parece un `Cargo.toml` mal nombrado; no se usa | **Muerto** |
| `webview.h` | Fragmento de librería; incluye headers inexistentes | **Muerto** |

La única UI vigente es `ui/`. No confundirla con los archivos de la raíz.

---

## 4. Compilación y ejecución

### Requisitos

- `g++` con C++17.
- GTK 3 y WebKit2GTK 4.1 (`pkg-config`).
- En Ubuntu/Debian: `build-essential pkg-config libgtk-3-dev libwebkit2gtk-4.1-dev`.

### Comandos

```bash
# Compilar
make

# Recompilar desde cero
make clean && make

# GUI
./lexlp

# CLI
./lexlp tests/prueba_basica.lp
./lexlp tests/prueba_completa.lp
./lexlp tests/prueba_errores.lp
./lexlp tests/prueba_limite.lp
```

**Importante:** el programa escribe en `output/` y carga `ui/index.html` usando rutas relativas al directorio de trabajo. Debe ejecutarse desde la raíz del repositorio. Si `output/` no existe, la escritura falla silenciosamente.

---

## 5. Estado actual del proyecto

**Rama:** `main` (sincronizada con `origin/main`).

**Última verificación:** 2026-11-12.

- Compila sin advertencias con `-Wall -Wextra`.
- Entorno verificado: `g++ 16.2.1`, GTK `3.24.52`, WebKitGTK `2.54.2`.
- No hay pruebas automatizadas ni comparación contra salidas esperadas.
- El árbol de trabajo ya tenía modificados `output/errores.txt`, `output/tabla_simbolos.txt` y `output/tokens.txt` antes de esta documentación. No asumir que esos archivos reflejan los tests oficiales.

### Resultados de los tests oficiales

| Prueba | Tokens | Símbolos | Errores | Resultado |
|---|---:|---:|---:|---|
| `prueba_basica.lp` | 32 | 3 | 0 | Correcto |
| `prueba_completa.lp` | 117 | 6 | 0 | Correcto |
| `prueba_errores.lp` | 34 | 7 | 6 | Detecta los 6 errores esperados |
| `prueba_limite.lp` | 80 | 10 | 5 | Detecta los 5 errores esperados |

La consolidación de errores funciona: `@variableInvalida`, `$precio`, `&`, `|`, `1.2.3`, `1..2`, `42.`, `123abc`, `1.2x` y cadenas sin cerrar se reportan como un único error léxico.

### Implementación del Entregable 1 (2026-11-12)

Se implementó el subconjunto inicial del analizador sintáctico:

- `src/Parser.h` / `src/Parser.cpp`: descenso recursivo para `PROGRAMA`,
  `LISTA_FUNCIONES`, `PRINCIPAL`, `BLOQUE`, declaraciones, asignaciones,
  expresiones, funciones top-level y llamadas con `func`.
- `KW_FUNC` agregado al lexer y a la UI.
- Integración en `src/main.cpp`: lexer → parser → JSON, CLI y GUI.
- Nuevos reportes: `output/errores_sintacticos.txt` y
  `output/resultado_sintactico.txt`.
- Nueva pestaña “Sintáctico” en la UI, con resultado y errores.
- `make test` para los casos de `tests/sintactico/`.
- Correcciones adicionales: BOM UTF-8, columnas en cadenas UTF-8, `\` + salto
  de línea, escape seguro de lexemas en la UI, creación/verificación de
  `output/`, `-MMD -MP` y `-Wextra`.
- `carlos-gif.gif` generado desde el video de `doc/`.

Resultado de `make test`: los 4 casos válidos son aceptados y los 5 inválidos
son rechazados con mensajes de línea/columna.

---

## 6. Problemas conocidos y prioridades

### Estado de correcciones (2026-11-12)

| Punto | Estado |
|---|---|
| 1. Salida dependiente del CWD | Parcial: se crea `output/` y se comprueban las escrituras; la UI sigue usando rutas relativas |
| 2. Carga de la UI dependiente del CWD | Pendiente |
| 3. `\` + salto de línea en cadenas | Resuelto |
| 4. Inyección HTML en la UI | Resuelto: se usan `textContent`/`createElement` |
| 5. CLI acoplado a GTK/WebKit | Pendiente |
| 6. Columnas UTF-8 | Parcial: corregidas dentro de cadenas; otros contextos siguen contando bytes |
| 7. BOM UTF-8 | Resuelto |
| 8. Argumentos y archivos vacíos | Resuelto: `--help`, `argc > 2`, archivo vacío y códigos de salida |
| 9. Pruebas automatizadas | Parcial: `make test` cubre el subconjunto sintáctico |
| 10. Artefactos generados versionados | Pendiente |
| 11. Archivos muertos en la raíz | Pendiente |
| 12. Rama `origin/bran` rota | Pendiente |
| 13. Makefile frágil | Parcial: `-MMD -MP`, `-Wextra` y target `test`; falta detección de WebKit 4.0 |
| 14. JSON manual | Pendiente: se mantiene el escapado manual |
| 15. Fallback de la UI sin WebKit | Pendiente |
| 16. CSS inválido | Resuelto |
| 17. Pruebas duplicadas en `ui/app.js` | Pendiente |
| 18. README desactualizado | Parcial: actualizado para el parser y los nuevos reportes |
| 19. EOF y lexema de `COMP` | Pendiente; confirmar con la cátedra |

### Alta prioridad

1. **Salida dependiente del directorio actual y fallo silencioso.**
   `src/main.cpp:41-59` escribe en `output/` con rutas relativas y no comprueba los `ofstream`. Si el directorio no existe, imprime éxito aunque no se haya guardado nada.

2. **Carga de la UI dependiente del CWD.**
   `src/main.cpp:115-121` carga `ui/index.html` de forma relativa. Ejecutar el binario desde otra carpeta deja la ventana vacía.

3. **Bug de línea/columna con `\` + salto de línea dentro de cadenas.**
   `src/Lexer.cpp:114-121` consume el `\n` sin actualizar `line` y permite cadenas multilínea. Un error posterior se reporta con una línea menos.

4. **Inyección HTML en la tabla de errores.**
   `ui/app.js:236-244` inserta `e.lexeme` con `innerHTML`. Una cadena sin cerrar que contenga `<img onerror=...>` se ejecuta en la UI. Usar `textContent`/`createElement`.

5. **CLI acoplado a GTK/WebKit.**
   `Makefile:2-3` y `src/main.cpp:3-4` obligan a tener librerías gráficas incluso para el modo CLI. Conviene separar núcleo, CLI y GUI.

### Prioridad media

6. **Columnas inconsistentes con UTF-8.** `src/Lexer.cpp:139` cuenta bytes en cadenas; una `ó` suma 2 columnas. Verificado: columna 30 reportada frente a 29 real.
7. **No se descarta BOM UTF-8.** Un archivo con BOM produce error y absorbe la primera palabra (`void`).
8. **Manejo incompleto de argumentos y archivos vacíos.** Un archivo vacío devuelve `1`; `argc > 2` abre la GUI en vez de mostrar error de uso.
9. **Sin pruebas automatizadas.** No hay `make test`, golden files ni CI.
10. **Artefactos generados versionados.** `output/*.txt` y `output/token.txt` están trackeados aunque `.gitignore` los ignore. La tabla actual contiene identificadores que no pertenecen a los tests oficiales.
11. **Archivos muertos y duplicados** en la raíz (`index.html`, `lexer_ui.*`, `argo.toml`, `webview.h`).
12. **Rama `origin/bran` rota.** Contiene marcadores de conflicto en `src/Lexer.cpp` y `ui/index.html`, además de un port a Rust con `tao`/`wry`. No fusionar sin resolver.
13. **Makefile frágil.** Sin dependencias de cabeceras, sin target `test`, y con WebKit 4.1 fijado aunque el README menciona 4.0.

### Prioridad baja

14. JSON generado manualmente; escapes incompletos para caracteres de control.
15. La UI no tiene fallback si `window.webkit.messageHandlers.ipc` no existe; los errores de IPC solo van a consola.
16. `ui/style.css:77` contiene `color: var(--);`, que es CSS inválido. El selector usa texto oscuro sobre fondo oscuro.
17. Las pruebas están duplicadas dentro de `ui/app.js` en lugar de cargarse desde `tests/`.
18. README desactualizado: menciona DFA, no incluye los archivos muertos y `ui/app.js:151` todavía dice “Invocación a Rust”.
19. No se emite token EOF; `COMP` pierde el lexema concreto (`<`, `<=`, etc.). Confirmar si la cátedra lo exige.

---

## 7. Backlog priorizado

### Completado en la iteración del Entregable 1

- Parser por descenso recursivo para el subconjunto inicial.
- Integración lexer → parser → CLI/GUI/JSON.
- Token `func` en lexer y UI.
- Reportes sintácticos y pestaña “Sintáctico”.
- `make test` con casos válidos e inválidos.
- Correcciones de BOM, UTF-8 en cadenas, `\` + newline, XSS y CSS.
- GIF `carlos-gif.gif`.

### P0 — Antes de entregar

1. Crear `output/` si no existe, comprobar cada escritura y fallar con mensaje claro.
2. Resolver rutas de `ui/` y `output/` respecto al ejecutable o a la raíz del proyecto.
3. Corregir `\` + salto de línea en cadenas.
4. Escapar todos los lexemas mostrados en la UI.
5. Descartar BOM UTF-8 al inicio.
6. Unificar el conteo de columnas para UTF-8.

### P1 — Calidad

7. Separar núcleo léxico, CLI y GUI.
8. Añadir `make test` con golden files para los cuatro casos.
9. Añadir pruebas de BOM, CRLF, UTF-8, archivo vacío y `\` + newline.
10. Añadir `-MMD -MP` y detección de WebKit 4.1/4.0.
11. Manejar `--help`, argumentos inválidos y archivos vacíos.

### P2 — Limpieza

12. Eliminar o archivar los archivos muertos de la raíz.
13. Sacar `output/*.txt` del índice y eliminar `output/token.txt`.
14. Resolver o abandonar formalmente `origin/bran`.
15. Actualizar README y `ui/app.js` (comentario de Rust).

---

## 8. Convenciones de código y salida

- C++17, cabeceras con guardas `#ifndef`.
- Mensajes de error y comentarios nuevos en español.
- Líneas y columnas: base 1.
- Posiciones de la tabla de símbolos: base 0.
- Formato de tokens:
  - identificadores: `<ID, pos>`
  - resto: `<TIPO>`
  - operadores: `<+>`, `<=>`, `<COMP>`, etc.
- Los errores léxicos no detienen el análisis.
- Las salidas deben ser deterministas y no incluir artefactos de ejecuciones manuales.
- Si se cambia el lexer, actualizar `tests/`, `README.md` y este archivo.

### Checklist antes de terminar una tarea

1. Leer `AGENTS.md`.
2. Revisar `git status`.
3. Compilar con `make`.
4. Ejecutar los cuatro tests oficiales.
5. Verificar `output/` si aplica.
6. Actualizar `AGENTS.md` con la entrada correspondiente.
7. No commitear `output/*.txt` ni binarios.

---

## 9. Registro de actividad

| Fecha | Hora | Zona | Modelo | Acción | Archivos |
|---|---|---|---|---|---|
| 2026-11-12 | 20:19:43 | -05 | DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`) | Creación de `AGENTS.md`. Se consolida el análisis del proyecto, el estado verificado, los hallazgos y el backlog priorizado. | `AGENTS.md` |
| 2026-11-12 | 20:20:50 | -05 | DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`) | Lectura de verificación posterior a la creación. Se actualiza la marca de “Última actualización” y se registra la lectura conforme a la regla principal. Sin cambios de contenido técnico. | `AGENTS.md` |
| 2026-11-12 | 20:41:12 | -05 | DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`) | Implementación del Entregable 1: parser por descenso recursivo, integración lexer/parser, token `func`, reportes sintácticos, pestaña de UI, `make test`, correcciones de BOM/UTF-8/backslash/XSS/CSS y generación de `carlos-gif.gif`. | `src/Parser.h`, `src/Parser.cpp`, `src/Lexer.cpp`, `src/Token.h`, `src/SymbolTable.*`, `src/main.cpp`, `Makefile`, `README.md`, `ui/*`, `tests/sintactico/*`, `carlos-gif.gif`, `AGENTS.md` |
| 2026-11-12 | 20:44:13 | -05 | DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`) | Verificación final: compilación con `-Wall -Wextra`, `make test` (4 válidos aceptados, 5 inválidos rechazados), validación de JSON y del GIF (`368×368`, 6 frames, 1.38 s, loop). Se actualiza el estado del backlog. | `AGENTS.md`, `README.md`, `carlos-gif.gif` |
| 2026-11-12 | 20:44:58 | -05 | DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`) | Ajuste final de `output/tokens.txt`: se eliminó el espacio final de la secuencia de tokens. Recompilación y verificación con `git diff --check`. | `src/main.cpp`, `AGENTS.md` |
| 2026-11-12 | 20:45:30 | -05 | DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`) | Reejecución final de `make test` (exit 0) y del caso válido `valido_minimo.lp` (exit 0); se regeneraron los archivos de `output/`. | `output/*.txt`, `AGENTS.md` |
| 2026-11-12 | 20:49:15 | -05 | DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`) | Se copió el GIF a `ui/carlos-gif.gif` y se incrustó en la esquina inferior derecha de la interfaz con posición fija, borde, sombra y bucle infinito. Se validaron HTML/JS y `make test`. | `ui/index.html`, `ui/style.css`, `ui/carlos-gif.gif`, `README.md`, `AGENTS.md` |
| 2026-11-12 | 20:58:05 | -05 | DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`) | Rediseño completo de la paleta de la UI: fondo azul profundo, superficies elevadas, texto de alto contraste, acentos turquesa, colores específicos para tokens, estados, tablas, botones, selector, scrollbars y GIF. Se validaron HTML/JS y se generaron capturas con Firefox headless. | `ui/style.css`, `AGENTS.md` |
| 2026-11-12 | 21:02:13 | -05 | DeepSeek V4.1 Flash 1017 (`opencode-go` / `deepseek-v4.1-flash`) | Creación de `actualizacion.md` con la documentación técnica completa del Entregable 1: alcance, arquitectura, cambios en lexer/parser, integración, pruebas, compilación, limitaciones y pendientes. | `actualizacion.md`, `AGENTS.md` |

**Plantilla para nuevas entradas:**

```markdown
| YYYY-MM-DD | HH:MM:SS | Zona | Modelo (`provider/model-id`) | Acción realizada y resultado | archivos afectados |
```

Si solo se leyó el archivo y no hubo cambios:

```markdown
| YYYY-MM-DD | HH:MM:SS | Zona | Modelo (`provider/model-id`) | Lectura/verificación sin cambios. | — |
```
