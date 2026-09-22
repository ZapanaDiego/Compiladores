#include "lexer.h"

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <algorithm>

// Equivalente a main.rs — punto de entrada con WebView usando GTK + WebKit2GTK

// ─── Utilidades ──────────────────────────────────────────────────────────

/// Lee el contenido completo de un archivo de texto.
static std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo: " << path << std::endl;
        return "";
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

/// Escapa una cadena para poder insertarla dentro de comillas simples en JS.
static std::string escape_for_js(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '\\': result += "\\\\"; break;
            case '\'': result += "\\'";  break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            default:   result += c;      break;
        }
    }
    return result;
}

// ─── Callback IPC: recibe el código fuente del JS y responde con JSON ────

static void on_script_message(WebKitUserContentManager* manager,
                               WebKitJavascriptResult* js_result,
                               gpointer user_data) {
    (void)manager;
    (void)user_data;

    JSCValue* value = webkit_javascript_result_get_js_value(js_result);
    if (!jsc_value_is_string(value)) return;

    gchar* source_code_raw = jsc_value_to_string(value);
    std::string source_code(source_code_raw);
    g_free(source_code_raw);

    // Ejecutar el análisis léxico
    LexerOutput output = Lexer::analyze(source_code);

    // Convertir resultado a JSON
    std::string json = output.to_json();

    // Enviar resultado de vuelta a JavaScript
    std::string script = "showResults('" + escape_for_js(json) + "')";

    WebKitWebView* webview = WEBKIT_WEB_VIEW(user_data);
    webkit_web_view_evaluate_javascript(webview, script.c_str(), -1,
                                        nullptr, nullptr, nullptr, nullptr);
}

// ─── Main ────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    gtk_init(&argc, &argv);

    // Crear ventana principal
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "LexLP - Fase 1: Lectura + Números");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    // Configurar el content manager para recibir mensajes desde JS
    WebKitUserContentManager* content_manager = webkit_user_content_manager_new();
    webkit_user_content_manager_register_script_message_handler(content_manager, "ipc");

    // Crear el WebView
    GtkWidget* webview = webkit_web_view_new_with_user_content_manager(content_manager);

    // Conectar el handler de mensajes IPC (pasamos el webview como user_data)
    g_signal_connect(content_manager, "script-message-received::ipc",
                     G_CALLBACK(on_script_message), webview);

    // Cargar el HTML desde el archivo ui/index.html
    std::string html_content = read_file("ui/index.html");
    if (!html_content.empty()) {
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(webview), html_content.c_str(), nullptr);
    }

    // Agregar el webview a la ventana
    gtk_container_add(GTK_CONTAINER(window), webview);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
