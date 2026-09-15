
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

```

---

## 🛠️ Requisitos e Instalación

### Prerrequisitos Nativos

1. **Rust & Cargo:** Tener instalado el toolchain oficial de Rust (versión 2021 o superior).
```bash
curl --proto '=https' --tlsv1.2 -sSf [https://sh.rustup.rs](https://sh.rustup.rs) | sh

```


2. **Librerías del Sistema (Linux / WebKitGTK):**
Si estás compilando en distribuciones basadas en Ubuntu/Debian, instala la dependencia de WebKitGTK necesaria para Webview:
```bash
sudo apt update
sudo apt install build-essential git pkg-config libwebkit2gtk-4.1-dev

```



---

## 🚀 Compilación y Ejecución

Para iniciar la aplicación en modo desarrollo:

```bash
# Clonar o ingresar al directorio del proyecto
cd LexLP

# Compilar y ejecutar
cargo run

```

Para generar un ejecutable compilado optimizado de producción:

```bash
cargo build --release

```

---

## 🔄 Flujo de Trabajo y Arquitectura

```
 +------------------------+              +------------------------+
 |   Interfaz Web (UI)    |              |   Backend en Rust      |
 |                        |              |                        |
 |  [ Editor de Código ]  |              |  [ Lexer::analyze ]    |
 |          |             |              |           |            |
 |    code  | (String)    |  analyzeCode |     parse | (Tokens)   |
 |          v             |  ----------> |           v            |
 |  window.analyzeCode()  |              |    serde_json::to_string()
 |          ^             |  <---------- |           |            |
 |    JSON  |             |  (Response)  |           v            |
 |          |             |              |  [ Return Result ]     |
 |  [ Tablas & Badges ]   |              |                        |
 +------------------------+              +------------------------+

```

---

## 📅 Hoja de Ruta (Roadmap por Fases)

* [x] **Semana 1 (Fase 1):** Lectura del código fuente, números enteros (`NUM_INT`), números decimales (`NUM_DEC`), reporte básico de errores, interfaz gráfica inicial.
* [ ] **Semana 2 (Fase 2):** Identificadores (`ID`), cadenas de texto (`TEXTO`), palabras reservadas (`int`, `float`, `void`, `if`, etc.) e implementación de la **Tabla de Símbolos**.
* [ ] **Semana 3 (Fase 3):** Operadores aritméticos, lógicos y relacionales, símbolos especiales (`{`, `}`, `;`), ignorar comentarios (`//`), exportación de resultados a la carpeta `output/` y casos de prueba completos.

```

```