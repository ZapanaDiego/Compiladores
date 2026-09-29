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

  // Colección de pruebas
  const testSamples = {
    standard: `int x = 3.5; // comentario\nprintln("ok");\n@`,
    basic: `int a = 10;\nfloat b = 20.5;\nint c = a + b * 2;`,
    complete: `int main() {\n    int arr[10];\n    int i = 0;\n    while (i < 10) {\n        if (i % 2 == 0 && i != 0) {\n            println("Par");\n        } else {\n            scanf("%d", &arr[i]);\n        }\n        i = i + 1;\n    }\n    return 0;\n}`,
    errors: `int x = @;\nfloat y = 1.2.3;\nstring s = "cadena sin cerrar;\nint z = & 5;`,
    edge_cases: `int _var_1 = 42.;\nint x = 1..2;\nint y = 123abc;\nfloat z = 1.2x;\nstring s = “hola mundo”;`
  };

  // Escapa caracteres de HTML para evitar deformaciones
  function escapeHtml(str) {
    if (str === null || str === undefined) return '';
    return String(str)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#039;');
  }

  // Actualizador de Posición de Cursor (Línea y Columna)
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

  // Manejo de Pestañas
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

  // Cargar Ejemplo de Prueba
  sampleSelect.addEventListener('change', (e) => {
    const key = e.target.value;
    if (key && testSamples[key]) {
      editor.value = testSamples[key];
      updateCursorPos();
    }
  });

  // Bridge de Comunicación con C++ (IPC WebKit)
  window.analyzeCode = function(code) {
    return new Promise((resolve) => {
      if (window.webkit && window.webkit.messageHandlers && window.webkit.messageHandlers.ipc) {
        window.showResults = function(jsonStr) {
          resolve(jsonStr);
        };
        window.webkit.messageHandlers.ipc.postMessage(code);
      } else {
        // Simulación para probar directamente en navegadores web convencionales
        console.warn('Entorno nativo WebKit no detectado. Modo simulación activado.');
        setTimeout(() => {
          resolve(JSON.stringify({
            tokens: [
              { type: "INT", lexeme: "int", attr: null, line: 1, col: 1 },
              { type: "ID", lexeme: "x", attr: 0, line: 1, col: 5 },
              { type: "ASSIGN", lexeme: "=", attr: null, line: 1, col: 7 },
              { type: "NUM_DEC", lexeme: "3.5", attr: null, line: 1, col: 9 },
              { type: "SEMICOLON", lexeme: ";", attr: null, line: 1, col: 12 }
            ],
            errors: [
              { line: 3, col: 1, lexeme: "@", message: "Simbolo no reconocido" }
            ],
            symbols: [
              { pos: 0, id: "x" }
            ]
          }));
        }, 150);
      }
    });
  };

  // Asignar clase de CSS por categoría de token
  function getTokenCategoryClass(type) {
    const typeUpper = String(type).toUpperCase();
    if (typeUpper === 'ID') return 'id-token';
    if (typeUpper.startsWith('NUM_') || typeUpper === 'NUMBER') return 'num-token';
    if (typeUpper.startsWith('KW_') || ['INT', 'FLOAT', 'CHAR', 'BOOLEAN', 'VOID', 'IF', 'ELSE', 'FOR', 'WHILE', 'SCANF', 'PRINTLN', 'MAIN', 'RETURN'].includes(typeUpper)) return 'kw-token';
    if (['TEXTO', 'STRING', 'STR'].includes(typeUpper)) return 'str-token';
    if (['ASSIGN', 'PLUS', 'MINUS', 'MULT', 'DIV', 'MOD', 'AND', 'OR', 'NOT', 'COMP', '=', '+', '-', '*', '/', '%', '&&', '||', '!'].includes(typeUpper)) return 'op-token';
    return 'sym-token';
  }

  // Formato de renderizado del token (<ID, 0> o <INT>)
  function formatTokenLabel(token) {
    if (token.type === 'ID' || (token.attr !== null && token.attr !== undefined && token.attr !== -1)) {
      const attrVal = token.attr !== null && token.attr !== undefined && token.attr !== -1 ? token.attr : (token.attribute ?? 0);
      return `&lt;ID, ${attrVal}&gt;`;
    }
    const cleanType = String(token.type).replace(/^KW_/, '');
    return `&lt;${escapeHtml(cleanType)}&gt;`;
  }

  // Módulo 1: Renderizar Tokens
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

  // Módulo 2: Renderizar Tabla de Símbolos
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

  // Módulo 3: Renderizar Errores
  function renderErrors(errors) {
    errorCount.textContent = errors.length;
    if (!errors || errors.length === 0) {
      errorsTableBody.innerHTML = '<tr><td colspan="4" class="empty-msg success-msg">✓ Sin errores léxicos.</td></tr>';
      return;
    }

    errorsTableBody.innerHTML = errors.map(err => `
      <tr>
        <td>${err.line}</td>
        <td>${err.col}</td>
        <td><code class="error-lexeme">${escapeHtml(err.lexeme)}</code></td>
        <td>${escapeHtml(err.message || 'ERROR_LEXICO')}</td>
      </tr>
    `).join('');
  }

  // Ejecución Principal
  async function executeAnalysis() {
    const code = editor.value;
    if (!code.trim()) return;

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

  // Carga inicial automática de muestra estándar
  if (!editor.value.trim()) {
    sampleSelect.value = 'standard';
    editor.value = testSamples.standard;
    updateCursorPos();
  }
});