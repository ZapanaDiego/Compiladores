/**
 * LexLP Studio - Analizador Léxico (Frontend JavaScript)
 * Compatible con:
 * 1. Entorno de escritorio WebKit2GTK (IPC con backend C++)
 * 2. Navegador web estándar (Motor léxico puro integrado en JS como fallback)
 */

document.addEventListener('DOMContentLoaded', () => {
  const editor = document.getElementById('codeEditor');
  const lineColCounter = document.getElementById('lineColCounter');
  const sampleSelect = document.getElementById('sampleSelect');
  const analyzeBtn = document.getElementById('analyzeBtn');
  const tabBtns = document.querySelectorAll('.tab-btn');
  const tabPanes = document.querySelectorAll('.tab-pane');

  const tokensContainer = document.getElementById('tokensContainer');
  const tokenCount = document.getElementById('tokenCount');
  const symbolTableBody = document.getElementById('symbolTableBody');
  const symbolCount = document.getElementById('symbolCount');
  const errorsTableBody = document.getElementById('errorsTableBody');
  const errorCount = document.getElementById('errorCount');

  // Muestras de prueba oficiales correspondientes a tests/
  const testSamples = {
    standard: `void main() {\n    int edad = 20;\n    float promedio = 15.5;\n    if (edad >= 18) {\n        println("Mayor de edad");\n    }\n    int @errorLexico = 99;\n    return;\n}`,
    basic: `// ==========================================\n// Prueba Básica: Expresiones Aritméticas y Variables\n// ==========================================\nvoid main() {\n    int valorA = 10;\n    int valorB = 25;\n    float total = valorA + valorB * 2.5;\n    println("Calculo finalizado exitosamente");\n    return;\n}`,
    complete: `// ==========================================\n// Prueba Completa: Cobertura Integral de Reglas Léxicas\n// ==========================================\nvoid main() {\n    int datos[10];\n    float promedio = 17.75;\n    boolean habilitado = 1;\n    char letra = 65;\n\n    // Estructuras de control y operadores relacionales/lógicos\n    int indice = 0;\n    while (indice < 10) {\n        datos[indice] = indice * 2;\n        indice = indice + 1;\n    }\n\n    for (int k = 0; k <= 5; k = k + 1) {\n        if (k % 2 == 0 && !habilitado || promedio >= 15.0) {\n            println("Condicion satisfecha");\n        } else {\n            scanf(datos[k]);\n        }\n    }\n\n    if (indice != 0) {\n        return;\n    }\n}`,
    errors: `// ==========================================\n// Prueba de Errores Léxicos\n// ==========================================\nvoid main() {\n    int @variableInvalida = 100;\n    float $precio = 45.99;\n    int errorAnd = a & b;\n    int errorOr = c | d;\n    float numeroInvalido = 1.2.3;\n    println("Cadena sin cerrar correctamente);\n    return;\n}`,
    edge_cases: `// ==========================================\n// Prueba de Casos Límite (Edge Cases)\n// ==========================================\nvoid main() {\n    // 1. Identificadores válidos con guiones bajos iniciales y números\n    int _contador = 1;\n    int __init__ = 0;\n    int _var123 = 50;\n\n    // 2. Errores numéricos: múltiples puntos y puntos sin decimales\n    float errPunto1 = 42.;\n    float errPuntosMultiples = 1..2;\n    float errTriple = 1.2.3.4;\n\n    // 3. Números pegados a letras (Inválidos)\n    int errNumId = 123abc;\n    float errDecId = 1.2x;\n\n    // 4. Cadenas válidas con comillas escapadas\n    println("Mensaje con \\"comillas\\" internas");\n\n    // 5. Operadores contiguos y comparaciones\n    int a = 10;\n    int b = 20;\n    if (a <= b && !(_contador != 0)) {\n        a = a + + b;\n    }\n\n    // 6. Comentario de cierre\n    return;\n}`
  };

  // Escapar HTML seguro
  function escapeHtml(str) {
    if (str === null || str === undefined) return '';
    return String(str)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#039;');
  }

  // Actualizador de línea y columna del cursor
  function updateCursorPos() {
    const text = editor.value.substring(0, editor.selectionStart);
    const lines = text.split('\n');
    const curLine = lines.length;
    const curCol = lines[lines.length - 1].length + 1;
    lineColCounter.textContent = `L: ${curLine} | C: ${curCol}`;
  }

  editor.addEventListener('keyup', updateCursorPos);
  editor.addEventListener('click', updateCursorPos);
  editor.addEventListener('input', updateCursorPos);
  document.addEventListener('selectionchange', () => {
    if (document.activeElement === editor) updateCursorPos();
  });

  // Atajo de teclado: Ctrl+Enter o Cmd+Enter para analizar
  editor.addEventListener('keydown', (e) => {
    if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
      e.preventDefault();
      executeAnalysis();
    }
  });

  // Manejo interactivo de pestañas
  tabBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      const targetTab = btn.getAttribute('data-tab');
      tabBtns.forEach(b => b.classList.remove('active'));
      tabPanes.forEach(p => p.classList.remove('active'));

      btn.classList.add('active');
      const activePane = document.getElementById(targetTab);
      if (activePane) activePane.classList.add('active');
    });
  });

  // Carga de programa de prueba al seleccionar en el menú
  sampleSelect.addEventListener('change', (e) => {
    const key = e.target.value;
    if (key && testSamples[key]) {
      editor.value = testSamples[key];
      updateCursorPos();
      executeAnalysis();
    }
  });

  // =========================================================================
  // MOTOR LÉXICO CLIENTE (Fallback fiel a src/Lexer.cpp y src/SymbolTable.cpp)
  // Permite ejecutar el análisis en cualquier navegador sin backend GTK/WebKit
  // =========================================================================
  function runJsLexer(input) {
    const keywords = {
      'int': 'INT',
      'float': 'FLOAT',
      'char': 'CHAR',
      'boolean': 'BOOLEAN',
      'void': 'VOID',
      'if': 'IF',
      'else': 'ELSE',
      'for': 'FOR',
      'while': 'WHILE',
      'scanf': 'SCANF',
      'println': 'PRINTLN',
      'main': 'MAIN',
      'return': 'RETURN'
    };

    function isTokenBoundary(ch) {
      return [';', ',', '(', ')', '{', '}', '[', ']', '+', '-', '*', '/', '%', '=', '<', '>', '!', '&', '|', '"'].includes(ch);
    }

    const tokens = [];
    const errors = [];
    const symbolsList = [];
    const symbolsMap = new Map();

    function insertOrGetSymbol(lexeme) {
      if (symbolsMap.has(lexeme)) {
        return symbolsMap.get(lexeme);
      }
      const idx = symbolsList.length;
      symbolsList.push(lexeme);
      symbolsMap.set(lexeme, idx);
      return idx;
    }

    let i = 0;
    const len = input.length;
    let line = 1;
    let col = 1;

    while (i < len) {
      const c = input[i];

      // Saltos de línea
      if (c === '\n') {
        line++;
        col = 1;
        i++;
        continue;
      }
      // Espacios en blanco
      if (c === '\r' || c === ' ' || c === '\t') {
        col++;
        i++;
        continue;
      }

      // Identificadores y Palabras Reservadas
      if (/[a-zA-Z_]/.test(c)) {
        const startCol = col;
        let ident = '';
        while (i < len && /[a-zA-Z0-9_]/.test(input[i])) {
          ident += input[i];
          i++;
          col++;
        }
        if (keywords.hasOwnProperty(ident)) {
          tokens.push({ type: keywords[ident], lexeme: ident, attr: null, line, col: startCol });
        } else {
          const attr = insertOrGetSymbol(ident);
          tokens.push({ type: 'ID', lexeme: ident, attr, line, col: startCol });
        }
        continue;
      }

      // Cadenas de texto ("..." o comillas tipográficas “...”)
      const isQuote = (c === '"' || c === '“' || c === '”');
      if (isQuote) {
        const startCol = col;
        const startLine = line;
        let strVal = '"';
        i++;
        col++;
        let closed = false;

        while (i < len) {
          const current = input[i];
          if (current === '\n') break;
          if (current === '\\') {
            strVal += '\\';
            i++; col++;
            if (i < len) {
              strVal += input[i];
              i++; col++;
            }
            continue;
          }
          if (current === '"' || current === '”' || current === '“') {
            strVal += '"';
            i++; col++;
            closed = true;
            break;
          }
          strVal += current;
          i++; col++;
        }

        if (closed) {
          tokens.push({ type: 'TEXTO', lexeme: strVal, attr: null, line: startLine, col: startCol });
        } else {
          errors.push({ line: startLine, col: startCol, lexeme: strVal, message: 'Cadena de texto no cerrada' });
        }
        continue;
      }

      // Constantes numéricas (Enteras y Decimales)
      if (/[0-9]/.test(c)) {
        const startCol = col;
        let numStr = '';
        let dotCount = 0;
        let hasInvalidChars = false;

        while (i < len) {
          const current = input[i];
          if (/[0-9]/.test(current)) {
            numStr += current;
            i++; col++;
          } else if (current === '.') {
            dotCount++;
            numStr += current;
            i++; col++;
          } else if (/[a-zA-Z_]/.test(current)) {
            hasInvalidChars = true;
            numStr += current;
            i++; col++;
          } else {
            break;
          }
        }

        if (dotCount > 1 || hasInvalidChars || numStr.endsWith('.')) {
          errors.push({ line, col: startCol, lexeme: numStr, message: 'Numero mal formado' });
        } else if (dotCount === 1) {
          tokens.push({ type: 'NUM_DEC', lexeme: numStr, attr: null, line, col: startCol });
        } else {
          tokens.push({ type: 'NUM_INT', lexeme: numStr, attr: null, line, col: startCol });
        }
        continue;
      }

      // Comentarios de una línea (//...\n) y Operador de división (/)
      if (c === '/') {
        if (i + 1 < len && input[i + 1] === '/') {
          i += 2;
          col += 2;
          while (i < len && input[i] !== '\n') {
            i++;
            col++;
          }
          if (i < len && input[i] === '\n') {
            line++;
            col = 1;
            i++;
          }
          continue;
        } else {
          tokens.push({ type: 'DIV', lexeme: '/', attr: null, line, col });
          i++; col++;
          continue;
        }
      }

      // Operador de asignación (=) y Operador relacional de igualdad (==)
      if (c === '=') {
        const startCol = col;
        if (i + 1 < len && input[i + 1] === '=') {
          tokens.push({ type: 'COMP', lexeme: '==', attr: null, line, col: startCol });
          i += 2; col += 2;
        } else {
          tokens.push({ type: 'ASSIGN', lexeme: '=', attr: null, line, col: startCol });
          i++; col++;
        }
        continue;
      }

      // Negación (!) y Desigualdad (!=)
      if (c === '!') {
        const startCol = col;
        if (i + 1 < len && input[i + 1] === '=') {
          tokens.push({ type: 'COMP', lexeme: '!=', attr: null, line, col: startCol });
          i += 2; col += 2;
        } else {
          tokens.push({ type: 'NOT', lexeme: '!', attr: null, line, col: startCol });
          i++; col++;
        }
        continue;
      }

      // Operadores relacionales (<, <=)
      if (c === '<') {
        const startCol = col;
        if (i + 1 < len && input[i + 1] === '=') {
          tokens.push({ type: 'COMP', lexeme: '<=', attr: null, line, col: startCol });
          i += 2; col += 2;
        } else {
          tokens.push({ type: 'COMP', lexeme: '<', attr: null, line, col: startCol });
          i++; col++;
        }
        continue;
      }

      // Operadores relacionales (>, >=)
      if (c === '>') {
        const startCol = col;
        if (i + 1 < len && input[i + 1] === '=') {
          tokens.push({ type: 'COMP', lexeme: '>=', attr: null, line, col: startCol });
          i += 2; col += 2;
        } else {
          tokens.push({ type: 'COMP', lexeme: '>', attr: null, line, col: startCol });
          i++; col++;
        }
        continue;
      }

      // Operador lógico AND (&&)
      if (c === '&') {
        const startCol = col;
        if (i + 1 < len && input[i + 1] === '&') {
          tokens.push({ type: 'AND', lexeme: '&&', attr: null, line, col: startCol });
          i += 2; col += 2;
        } else {
          errors.push({ line, col: startCol, lexeme: '&', message: 'Simbolo no reconocido' });
          i++; col++;
        }
        continue;
      }

      // Operador lógico OR (||)
      if (c === '|') {
        const startCol = col;
        if (i + 1 < len && input[i + 1] === '|') {
          tokens.push({ type: 'OR', lexeme: '||', attr: null, line, col: startCol });
          i += 2; col += 2;
        } else {
          errors.push({ line, col: startCol, lexeme: '|', message: 'Simbolo no reconocido' });
          i++; col++;
        }
        continue;
      }

      // Operadores aritméticos (+, -, *, %)
      if (c === '+') { tokens.push({ type: 'PLUS', lexeme: '+', attr: null, line, col }); i++; col++; continue; }
      if (c === '-') { tokens.push({ type: 'MINUS', lexeme: '-', attr: null, line, col }); i++; col++; continue; }
      if (c === '*') { tokens.push({ type: 'MULT', lexeme: '*', attr: null, line, col }); i++; col++; continue; }
      if (c === '%') { tokens.push({ type: 'MOD', lexeme: '%', attr: null, line, col }); i++; col++; continue; }

      // Delimitadores y signos de puntuación
      if (c === '(') { tokens.push({ type: 'LPAREN', lexeme: '(', attr: null, line, col }); i++; col++; continue; }
      if (c === ')') { tokens.push({ type: 'RPAREN', lexeme: ')', attr: null, line, col }); i++; col++; continue; }
      if (c === '[') { tokens.push({ type: 'LBRACKET', lexeme: '[', attr: null, line, col }); i++; col++; continue; }
      if (c === ']') { tokens.push({ type: 'RBRACKET', lexeme: ']', attr: null, line, col }); i++; col++; continue; }
      if (c === '{') { tokens.push({ type: 'LBRACE', lexeme: '{', attr: null, line, col }); i++; col++; continue; }
      if (c === '}') { tokens.push({ type: 'RBRACE', lexeme: '}', attr: null, line, col }); i++; col++; continue; }
      if (c === ',') { tokens.push({ type: 'COMMA', lexeme: ',', attr: null, line, col }); i++; col++; continue; }
      if (c === ';') { tokens.push({ type: 'SEMICOLON', lexeme: ';', attr: null, line, col }); i++; col++; continue; }

      // Captura de símbolos y variables inválidas completas (ej: @variableInvalida, $precio)
      const startCol = col;
      let errLexeme = c;
      i++;
      col++;

      while (i < len) {
        const next = input[i];
        if (next === ' ' || next === '\t' || next === '\r' || next === '\n' || isTokenBoundary(next)) {
          break;
        }
        errLexeme += next;
        i++;
        col++;
      }

      errors.push({ line, col: startCol, lexeme: errLexeme, message: 'Simbolo no reconocido' });
    }

    const symbols = symbolsList.map((id, pos) => ({ pos, id }));

    return { tokens, errors, symbols };
  }

  // =========================================================================
  // Bridge de Comunicación con Backend o Modo Navegador
  // =========================================================================
  window.analyzeCode = function(code) {
    return new Promise((resolve) => {
      if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.ipc) {
        window.showResults = function(jsonStr) {
          resolve(jsonStr);
        };
        window.webkit.messageHandlers.ipc.postMessage(code);
      } else {
        // En navegador estándar, usamos el motor JS síncrono / microtarea
        const results = runJsLexer(code);
        resolve(results);
      }
    });
  };

  // Asignar clase de estilo CSS por categoría de token
  function getTokenCategoryClass(type) {
    const t = String(type).toUpperCase();
    if (t === 'ID') return 'id-token';
    if (t.startsWith('NUM_') || t === 'NUMBER') return 'num-token';
    if (t.startsWith('KW_') || ['INT', 'FLOAT', 'CHAR', 'BOOLEAN', 'VOID', 'IF', 'ELSE', 'FOR', 'WHILE', 'SCANF', 'PRINTLN', 'MAIN', 'RETURN'].includes(t)) return 'kw-token';
    if (['TEXTO', 'STRING', 'STR'].includes(t)) return 'str-token';
    if (['ASSIGN', 'PLUS', 'MINUS', 'MULT', 'DIV', 'MOD', 'AND', 'OR', 'NOT', 'COMP', '=', '+', '-', '*', '/', '%', '&&', '||', '!'].includes(t)) return 'op-token';
    return 'sym-token';
  }

  // Formato oficial de salida para la etiqueta del token (<ID, 0> o <INT>, <=>, etc.)
  function formatTokenLabel(token) {
    if (token.type === 'ID') {
      const attrVal = (token.attr !== null && token.attr !== undefined) ? token.attr : 0;
      return `&lt;ID, ${attrVal}&gt;`;
    }
    
    // Asignación de tipos simbólicos a representación exigida
    const mapRepr = {
      'ASSIGN': '=',
      'PLUS': '+',
      'MINUS': '-',
      'MULT': '*',
      'DIV': '/',
      'MOD': '%',
      'AND': '&&',
      'OR': '||',
      'NOT': '!',
      'LPAREN': '(',
      'RPAREN': ')',
      'LBRACKET': '[',
      'RBRACKET': ']',
      'LBRACE': '{',
      'RBRACE': '}',
      'COMMA': ',',
      'SEMICOLON': ';'
    };

    const cleanType = mapRepr[token.type] || String(token.type).replace(/^KW_/, '');
    return `&lt;${escapeHtml(cleanType)}&gt;`;
  }

  // Renderizar Lista de Tokens
  function renderTokens(tokens) {
    tokenCount.textContent = tokens.length;
    if (!tokens || tokens.length === 0) {
      tokensContainer.innerHTML = '<p class="empty-msg">No se generaron tokens.</p>';
      return;
    }

    tokensContainer.innerHTML = tokens.map(token => {
      const catClass = getTokenCategoryClass(token.type);
      const label = formatTokenLabel(token);
      const titleAttr = `Lexema: "${escapeHtml(token.lexeme)}"\nPosición: L:${token.line} C:${token.col}`;
      return `<span class="token-tag ${catClass}" title="${titleAttr}">${label}</span>`;
    }).join('');
  }

  // Renderizar Tabla de Símbolos
  function renderSymbolTable(symbols) {
    symbolCount.textContent = symbols.length;
    if (!symbols || symbols.length === 0) {
      symbolTableBody.innerHTML = '<tr><td colspan="2" class="empty-msg">No hay identificadores.</td></tr>';
      return;
    }

    symbolTableBody.innerHTML = symbols.map(sym => `
      <tr>
        <td><strong>${sym.pos}</strong></td>
        <td><code>${escapeHtml(sym.id)}</code></td>
      </tr>
    `).join('');
  }

  // Renderizar Errores Léxicos
  function renderErrors(errors) {
    errorCount.textContent = errors.length;
    if (!errors || errors.length === 0) {
      errorsTableBody.innerHTML = '<tr><td colspan="4" class="empty-msg success-msg">✓ Sin errores léxicos detectados.</td></tr>';
      return;
    }

    errorsTableBody.innerHTML = errors.map(err => `
      <tr>
        <td>${err.line}</td>
        <td>${err.col}</td>
        <td><code class="error-lexeme">${escapeHtml(err.lexeme)}</code></td>
        <td><b>ERROR_LEXICO</b> (${escapeHtml(err.message || 'Símbolo inválido')})</td>
      </tr>
    `).join('');
  }

  // Ejecución Principal
  async function executeAnalysis() {
    const code = editor.value;
    if (!code) {
      renderTokens([]);
      renderSymbolTable([]);
      renderErrors([]);
      return;
    }

    try {
      const rawResponse = await window.analyzeCode(code);
      const data = typeof rawResponse === 'string' ? JSON.parse(rawResponse) : rawResponse;

      renderTokens(data.tokens || []);
      renderSymbolTable(data.symbols || []);
      renderErrors(data.errors || []);
    } catch (err) {
      console.error('Error al procesar el análisis:', err);
    }
  }

  analyzeBtn.addEventListener('click', executeAnalysis);

  // Inicialización: cargar programa de demostración estándar y ejecutar análisis
  if (!editor.value.trim()) {
    sampleSelect.value = 'standard';
    editor.value = testSamples.standard;
    updateCursorPos();
  }
  executeAnalysis();
});