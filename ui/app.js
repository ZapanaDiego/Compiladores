// Bridge de comunicación con C++
window.analyzeCode = function(code) {
  return new Promise((resolve) => {
    window.showResults = function(jsonStr) {
      resolve(jsonStr);
    };
    window.webkit.messageHandlers.ipc.postMessage(code);
  });
};

// Manejo de pestañas
document.querySelectorAll('.tab-btn').forEach(btn => {
  btn.addEventListener('click', () => {
    document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
    document.querySelectorAll('.tab-pane').forEach(p => p.classList.remove('active'));
    
    btn.classList.add('active');
    document.getElementById(btn.dataset.tab).classList.add('active');
  });
});

// Contador de línea y columna en el editor
const editor = document.getElementById('codeEditor');
const lineColCounter = document.getElementById('lineColCounter');

editor.addEventListener('keyup', updateCursorPos);
editor.addEventListener('click', updateCursorPos);

function updateCursorPos() {
  const text = editor.value.substring(0, editor.selectionStart);
  const lines = text.split('\n');
  const line = lines.length;
  const col = lines[lines.length - 1].length + 1;
  lineColCounter.textContent = `L: ${line} | C: ${col}`;
}

// Colección de pruebas disponibles correspondientes a tests/
const testSamples = {
  syntax_valid: `
// ==========================================
// Sintaxis válida para el Entregable 1
// ==========================================
void main() {
    int edad;
    float promedio;
    edad = 20;
    promedio = 15.5;
}`,
  syntax_invalid: `
// ==========================================
// Sintaxis inválida: falta un punto y coma
// ==========================================
void main() {
    int edad
    edad = 20;
}`,
  basic: `
// ==========================================
// Prueba Básica: Expresiones Aritméticas y Variables
// ==========================================
void main() {
    int valorA = 10;
    int valorB = 25;
    float total = valorA + valorB * 2.5;
    println("Calculo finalizado exitosamente");
    return;
}`,
  complete: `
// ==========================================
// Prueba Completa: Cobertura Integral de Reglas Léxicas
// ==========================================
void main() {
    int datos[10];
    float promedio = 17.75;
    boolean habilitado = 1;
    char letra = 65;

    // Estructuras de control y operadores relacionales/lógicos
    int indice = 0;
    while (indice < 10) {
        datos[indice] = indice * 2;
        indice = indice + 1;
    }

    for (int k = 0; k <= 5; k = k + 1) {
        if (k % 2 == 0 && !habilitado || promedio >= 15.0) {
            println("Condicion satisfecha");
        } else {
            scanf(datos[k]);
        }
    }

    if (indice != 0) {
        return;
    }
}`,
  errors: `
// ==========================================
// Prueba de Errores Léxicos
// ==========================================
void main() {
    int @variableInvalida = 100;
    float $precio = 45.99;
    int errorAnd = a & b;
    int errorOr = c | d;
    float numeroInvalido = 1.2.3;
    println("Cadena sin cerrar correctamente);
    return;
}`,
  edge_cases: `
// ==========================================
// Prueba de Casos Límite (Edge Cases)
// ==========================================
void main() {
    // 1. Identificadores válidos con guiones bajos iniciales y números
    int _contador = 1;
    int __init__ = 0;
    int _var123 = 50;

    // 2. Errores numéricos: múltiples puntos y puntos sin decimales
    float errPunto1 = 42.;
    float errPuntosMultiples = 1..2;
    float errTriple = 1.2.3.4;

    // 3. Números pegados a letras (Inválidos)
    int errNumId = 123abc;
    float errDecId = 1.2x;

    // 4. Cadenas válidas con comillas escapadas
    println("Mensaje con \\"comillas\\" internas");

    // 5. Operadores contiguos y comparaciones
    int a = 10;
    int b = 20;
    if (a <= b && !(_contador != 0)) {
        a = a + + b;
    }

    // 6. Comentario de cierre
    return;
}`,
  standard: `void main() {
    int edad = 20;
    float promedio = 15.5;
    if (edad >= 18) {
        println("Mayor de edad");
    }
    int @errorLexico = 99;
    return;
}`
};

// Cargar prueba seleccionada desde el desplegable
function loadSelectedSample() {
  const select = document.getElementById('sampleSelect');
  const selectedKey = select.value;
  if (testSamples[selectedKey]) {
    editor.value = testSamples[selectedKey];
    updateCursorPos();
  }
}

// Ejecución del análisis y renderizado modular
async function executeAnalysis() {
  const code = editor.value;
  if (!code.trim()) return;

  try {
    // Invocación al backend C++ vía WebView
    const rawResponse = await window.analyzeCode(code);
    const data = JSON.parse(rawResponse);

    renderTokens(data.tokens || []);
    renderSymbolTable(data.symbols || []);
    renderErrors(data.errors || []);
    renderSyntax(data.syntax || {});
  } catch (err) {
    console.error("Error al procesar la comunicación con C++:", err);
    const status = document.getElementById('syntaxResult');
    if (status) {
      status.className = 'syntax-result syntax-rejected';
      status.textContent = 'No se pudo obtener el resultado del analizador.';
    }
  }
}

// Módulo 1: Renderizado de Tokenss
function renderTokens(tokens) {
  const container = document.getElementById('tokensContainer');
  document.getElementById('tokenCount').textContent = tokens.length;
  container.innerHTML = '';

  if (tokens.length === 0) {
    container.innerHTML = '<div class="empty-state">No se reconocieron tokens.</div>';
    return;
  }

  tokens.forEach(t => {
    const tag = document.createElement('div');
    const isID = t.type === 'ID';
    const isNum = t.type === 'NUM_INT' || t.type === 'NUM_DEC';
    const isKw = ['INT','FLOAT','CHAR','BOOLEAN','VOID','IF','ELSE','FOR','WHILE','SCANF','PRINTLN','MAIN','RETURN','FUNC'].includes(t.type);
    const isOp = ['=', '+', '-', '*', '/', '%', '&&', '||', '!', 'COMP'].includes(t.type);
    const isSym = ['(', ')', '[', ']', '{', '}', ',', ';'].includes(t.type);
    const isStr = t.type === 'TEXTO';
    
    let extraClass = '';
    if (isID) extraClass = 'id-token';
    else if (isNum) extraClass = 'num-token';
    else if (isKw) extraClass = 'kw-token';
    else if (isOp) extraClass = 'op-token';
    else if (isSym) extraClass = 'sym-token';
    else if (isStr) extraClass = 'str-token';

    tag.className = `token-tag ${extraClass}`;
    tag.title = `Lexema: ${t.lexeme} | Línea: ${t.line}, Columna: ${t.col}`;

    // Formato de salida exigido: <ID, pos> o <TIPO>
    tag.textContent = t.attr !== null && t.attr !== undefined 
      ? `<${t.type}, ${t.attr}>` 
      : `<${t.type}>`;
      
    container.appendChild(tag);
  });
}

// Módulo 2: Renderizado de Tabla de Símbolos
function renderSymbolTable(symbols) {
  const tbody = document.getElementById('symbolTableBody');
  document.getElementById('symbolCount').textContent = symbols.length;
  tbody.innerHTML = '';

  if (symbols.length === 0) {
    tbody.innerHTML = '<tr><td colspan="2" class="empty-cell">No hay identificadores.</td></tr>';
    return;
  }

  symbols.forEach(s => {
    const tr = document.createElement('tr');

    const tdPos = document.createElement('td');
    const strong = document.createElement('strong');
    strong.textContent = s.pos;
    tdPos.appendChild(strong);

    const tdId = document.createElement('td');
    const code = document.createElement('code');
    code.textContent = s.id;
    tdId.appendChild(code);

    tr.appendChild(tdPos);
    tr.appendChild(tdId);
    tbody.appendChild(tr);
  });
}

// Módulo 3: Renderizado de Errores Léxicos
function renderErrors(errors) {
  const tbody = document.getElementById('errorsTableBody');
  document.getElementById('errorCount').textContent = errors.length;
  tbody.innerHTML = '';

  if (errors.length === 0) {
    tbody.innerHTML = '<tr><td colspan="4" class="empty-cell">✓ Sin errores léxicos.</td></tr>';
    return;
  }

  errors.forEach(e => {
    const tr = document.createElement('tr');

    const tdLine = document.createElement('td');
    tdLine.textContent = e.line;
    const tdCol = document.createElement('td');
    tdCol.textContent = e.col;
    const tdLexeme = document.createElement('td');
    const code = document.createElement('code');
    code.textContent = e.lexeme;
    tdLexeme.appendChild(code);
    const tdResult = document.createElement('td');
    const strong = document.createElement('strong');
    strong.textContent = 'ERROR_LEXICO';
    tdResult.appendChild(strong);

    tr.appendChild(tdLine);
    tr.appendChild(tdCol);
    tr.appendChild(tdLexeme);
    tr.appendChild(tdResult);
    tbody.appendChild(tr);
  });
}

// Módulo 4: Renderizado del Resultado Sintáctico
function renderSyntax(syntax) {
  const status = document.getElementById('syntaxResult');
  const tbody = document.getElementById('syntaxTableBody');
  const counter = document.getElementById('syntaxCount');
  if (!status || !tbody || !counter) return;

  status.className = 'syntax-result';
  tbody.innerHTML = '';

  if (!syntax || syntax.executed === false) {
    status.classList.add('syntax-skipped');
    status.textContent = 'Análisis sintáctico no ejecutado: corrige primero los errores léxicos.';
    counter.textContent = '—';
    tbody.innerHTML = '<tr><td colspan="5" class="empty-cell">Sin análisis sintáctico.</td></tr>';
    return;
  }

  const errors = syntax.errors || [];
  counter.textContent = errors.length;

  if (syntax.accepted) {
    status.classList.add('syntax-accepted');
    status.textContent = '✓ Programa aceptado sintácticamente.';
    tbody.innerHTML = '<tr><td colspan="5" class="empty-cell">Sin errores sintácticos.</td></tr>';
    return;
  }

  status.classList.add('syntax-rejected');
  status.textContent = '✗ Programa rechazado: se encontraron errores sintácticos.';
  if (errors.length === 0) {
    tbody.innerHTML = '<tr><td colspan="5" class="empty-cell">El parser no reportó detalles.</td></tr>';
    return;
  }

  errors.forEach(e => {
    const tr = document.createElement('tr');
    [e.line, e.col, e.found, e.expected, e.message].forEach(value => {
      const td = document.createElement('td');
      td.textContent = value;
      tr.appendChild(td);
    });
    tbody.appendChild(tr);
  });
}