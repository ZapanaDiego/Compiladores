# LexLP — Analizador Léxico (Rust + Web Interface)

**LexLP** es un analizador léxico desarrollado en **Rust** para un lenguaje imperativo simplificado denominado **LP** (inspirado en lenguajes como C y Java). 

A diferencia de la implementación por consola tradicional, este proyecto incorpora una interfaz visual moderna basada en **HTML/CSS/JS** dentro de un binario nativo utilizando **Webview**, permitiendo probar código fuente en tiempo real, visualizar tokens y ubicar errores léxicos por línea y columna.

---

## 📌 Estado del Proyecto: Fase 1

Actualmente el proyecto se encuentra en la **Fase 1** de desarrollo:

- [x] **Lectura del código fuente:** Recepción e iteración de cadenas desde el editor Web.
- [x] **Mapeo de Líneas y Columnas:** Control preciso de la ubicación de los caracteres.
- [x] **Reconocimiento de Números Enteros (`NUM_INT`):** Secuencia de dígitos (`[0-9]+`).
- [x] **Reconocimiento de Números Decimales (`NUM_DEC`):** Secuencia de dígitos con punto decimal (`[0-9]+\.[0-9]+`).
- [x] **Manejo de Errores Léxicos (Fase 1):** Detección y reporte de símbolos no reconocidos.
- [x] **Integración Rust-JS:** Comunicación asíncrona mediante JSON serializado con `Serde`.

---

## 📂 Estructura del Proyecto

El proyecto sigue las convenciones de Cargo en Rust combinadas con un contenedor estático para la interfaz gráfica:

```text
LexLP/
├── Cargo.toml          # Configuración de dependencias (serde, webview)
├── README.md            # Documentación del proyecto
├── ui/
│   └── index.html       # Interfaz gráfica (HTML, CSS y JS empacados)
├── src/
│   ├── main.rs          # Punto de entrada y binding con Webview
│   ├── token.rs         # Modelos de datos (Token, LexerError, LexerOutput)
│   └── lexer.rs         # Lógica del analizador léxico (Fase 1)
├── tests/               # Archivos de prueba (.lp)
│   ├── prueba_basica.lp
│   ├── prueba_completa.lp
│   └── prueba_errores.lp
└── output/              # Salidas en texto para reportes futuros
    ├── tokens.txt
    ├── tabla_simbolos.txt
    └── errores.txt