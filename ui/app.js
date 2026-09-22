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

// Cargar ejemplo estándar del documento LP
function loadSampleCode() {
  editor.value = `void main() {
    int edad = 20;
    float promedio = 15.5;
    if (edad >= 18) {
        println("Mayor de edad");
    }
    int @errorLexico = 99;
    return;
}`;
  updateCursorPos();
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
    console.error("Error al procesar la comunicación con Rust:", err);
  }
}

// Módulo 1: Renderizado de Tokens
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
    
    tag.className = `token-tag ${isID ? 'id-token' : ''} ${isNum ? 'num-token' : ''}`;
    // Formato de salida exigido: <ID, pos> o <TIPO>[cite: 3]
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

  async function cargarResultados() {
  const res = await fetch("../output/data.json");
  const data = await res.json();
  renderSymbolTable(data.symbol_table);
}

function renderSymbolTable(symbolTable) {
  const body = document.getElementById("symbol-table-body");
  body.innerHTML = "";
  symbolTable.forEach(entry => {
    const row = document.createElement("tr");
    row.classList.add(cssClassFor(entry.type));
    row.innerHTML = `
      <td>${entry.name}</td>
      <td>${entry.type}</td>
      <td>${entry.line}:${entry.col}</td>
      <td>${entry.occurrences}</td>
    `;
    body.appendChild(row);
  });
}

function cssClassFor(type) {
  switch (type) {
    case "KEYWORD": return "tok-keyword";
    case "ID": return "tok-id";
    case "NUMBER": return "tok-number";
    case "STRING": return "tok-string";
    case "ERROR": return "tok-error";
    default: return "";
  }
}

cargarResultados();
}