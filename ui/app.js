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
  basic: `// ==========================================
// Prueba Básica: Expresiones Aritméticas y Variables
// ==========================================
void main() {
    int valorA = 10;
    int valorB = 25;
    float total = valorA + valorB * 2.5;
    println("Calculo finalizado exitosamente");
    return;
}`,
  complete: `// ==========================================
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
  errors: `// ==========================================
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
  edge_cases: `// ==========================================
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
    // Invocación a Rust vía Webview
    const rawResponse = await window.analyzeCode(code);
    const data = JSON.parse(rawResponse);

    renderTokens(data.tokens || []);
    renderSymbolTable(data.symbols || []);
    renderErrors(data.errors || []);
  } catch (err) {
    console.error("Error al procesar la comunicación con c++:", err);
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
    const isKw = ['INT','FLOAT','CHAR','BOOLEAN','VOID','IF','ELSE','FOR','WHILE','SCANF','PRINTLN','MAIN','RETURN'].includes(t.type);
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
    tbody.innerHTML += `
      <tr>
        <td><b>${s.pos}</b></td>
        <td><code>${s.id}</code></td>
      </tr>
    `;
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
    tbody.innerHTML += `
      <tr>
        <td>${e.line}</td>
        <td>${e.col}</td>
        <td><code>${e.lexeme}</code></td>
        <td><b>ERROR_LEXICO</b></td>
      </tr>
    `;
  });
}