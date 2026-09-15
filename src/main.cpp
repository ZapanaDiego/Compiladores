#include "Lexer.h"
#include <nlohmann/json.hpp>
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

using json = nlohmann::json;

// Widget global del WebView para poder llamar eval desde el callback
static WebKitWebView *g_webview = nullptr;

// Lee el contenido de un archivo y lo retorna como string
static std::string readFile(const std::string &filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir " << filepath << std::endl;
        return "";
    }
    std::stringstream buf;
    buf << file.rdbuf();
    return buf.str();
}

// Escapa un string para incrustarlo de forma segura dentro de un literal JS
static std::string jsEscape(const std::string &s) {
    std::string out;
    out.reserve(s.size() + 64);
    for (unsigned char c : s) {
        if (c == '\\') out += "\\\\";
        else if (c == '\'') out += "\\'";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\0') out += "\\0";
        else out += c;
    }
    return out;
}

// Callback que se ejecuta cuando JS llama a window.webkit.messageHandlers.external.postMessage(msg)
static void on_message(WebKitUserContentManager *,
                       WebKitJavascriptResult *result,
                       gpointer) {
    // Obtener el string enviado desde JS
    JSCValue *value = webkit_javascript_result_get_js_value(result);
    char *raw = jsc_value_to_string(value);
    if (!raw) return;

    std::string msg(raw);
    g_free(raw);

    // Deserializar: JS envía { id, code }
    json req;
    try {
        req = json::parse(msg);
    } catch (...) {
        std::cerr << "Error: JSON inválido desde JS\n";
        return;
    }

    std::string callId   = req.value("id", "");
    std::string code     = req.value("code", "");

    // Ejecutar análisis léxico
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();

    json result_json = json::array();
    for (const auto &tok : tokens) {
        result_json.push_back({
            {"tipo",    tok.type},
            {"lexema",  tok.lexeme},
            {"linea",   tok.line},
            {"columna", tok.column}
        });
    }

    // Devolver resultado a JS via eval
    std::string payload = jsEscape(result_json.dump());
    std::string js = "window.__lexlpResolve('" + jsEscape(callId) + "', JSON.parse('" + payload + "'));";

    webkit_web_view_evaluate_javascript(g_webview, js.c_str(), -1, nullptr, nullptr, nullptr, nullptr, nullptr);
}

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // --- Ventana principal ---
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "LexLP: Analizador Léxico (Fase 1)");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 650);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    // --- Gestor de contenido de usuario (para el canal de mensajes JS->C++) ---
    WebKitUserContentManager *manager = webkit_user_content_manager_new();
    webkit_user_content_manager_register_script_message_handler(manager, "external");
    g_signal_connect(manager, "script-message-received::external",
                     G_CALLBACK(on_message), nullptr);

    // --- WebView ---
    g_webview = WEBKIT_WEB_VIEW(webkit_web_view_new_with_user_content_manager(manager));

    // Incrustar el canal de comunicación JS->C++ antes de que cargue la página
    const char *bridgeScript =
        "window.__lexlpCallbacks = {};"
        "window.analizarCodigo = function(code) {"
        "  return new Promise(function(resolve, reject) {"
        "    var id = Math.random().toString(36).substr(2);"
        "    window.__lexlpCallbacks[id] = { resolve: resolve, reject: reject };"
        "    window.webkit.messageHandlers.external.postMessage(JSON.stringify({ id: id, code: code }));"
        "  });"
        "};"
        "window.__lexlpResolve = function(id, tokens) {"
        "  if (window.__lexlpCallbacks[id]) {"
        "    window.__lexlpCallbacks[id].resolve(tokens);"
        "    delete window.__lexlpCallbacks[id];"
        "  }"
        "};";

    WebKitUserScript *script = webkit_user_script_new(
        bridgeScript,
        WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
        WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
        nullptr, nullptr);
    webkit_user_content_manager_add_script(manager, script);
    webkit_user_script_unref(script);

    // Cargar index.html
    std::string html = readFile("index.html");
    if (html.empty()) {
        html = "<h1 style='color:red;font-family:monospace'>Error: no se encontró index.html</h1>";
    }
    webkit_web_view_load_html(g_webview, html.c_str(), "file:///");

    // Poner el WebView dentro de la ventana
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(g_webview));
    gtk_widget_show_all(window);

    gtk_main();
    return 0;
}
