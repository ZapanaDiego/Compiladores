### Instrucciones de Compilación y Ejecución (CachyOS)

#### 1. Instalar Dependencias
Necesitas el compilador, las librerías base de `webkit2gtk-4.1` (backend nativo), la librería JSON y el header solitario de `webview.h`. Ejecuta los siguientes comandos en tu terminal:

```bash
# 1. Instalar herramientas de compilación, json y webkit2gtk
sudo pacman -S --needed gcc make pkgconf webkit2gtk nlohmann-json wget

# 2. Descargar la biblioteca header-only webview.h en el directorio del proyecto
wget -O webview.h https://raw.githubusercontent.com/webview/webview/master/webview.h
```

#### 2. Compilar el Proyecto
Utilizamos `g++` asegurándonos del estándar C++17 y de pasar los flags adecuados que requiere el backend gráfico:

```bash
g++ -std=c++17 main.cpp Lexer.cpp -o lexlp $(pkg-config --cflags --libs webkit2gtk-4.1)
```

#### 3. Ejecutar la Aplicación
Una vez finalizada la compilación, se generará tu binario `lexlp`. Únicamente debes ejecutarlo (asegúrate de que `index.html` siga en la misma carpeta que ejecutas el binario).

```bash
./lexlp
```

### Notas sobre la Implementación de la Fase 1
* **Tokens Aceptados:** Por ahora identificará `NUM_INT` (`[0-9]+`) y `NUM_DEC` (`[0-9]+\.[0-9]+`).
* **Caracteres Desconocidos:** Para prevenir bucles infinitos frente a cualquier otro carácter, lo etiquetará como `DESCONOCIDO` (en rojo en la UI) y continuará analizando. 
* **Posicionamiento Geográfico:** La función `skipWhitespace()` está controlada para que un salto de línea `\n` siempre sume 1 a `Línea` y formatee la `Columna` a 1 de forma precisa para el inicio de un Token.