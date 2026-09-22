#include "Lexer.h"
#include "SymbolTable.h"
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

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

static void execute_analysis_and_export(const std::string& source_code, std::string& json_output) {
    SymbolTable symTable;
    LexerOutput output = Lexer::analyze(source_code, symTable);
    
    // Archivo de Tokens (Formato exigido por la entrega)
    std::ofstream out_tokens("output/tokens.txt");
    if (out_tokens.is_open()) {
        for (const auto& t : output.tokens) {
            out_tokens << t.toString() << " ";
        }
        out_tokens << "\n";
    }

    // Tabla de Símbolos (Formato exigido por la entrega)
    symTable.exportToFile("output/tabla_simbolos.txt");

    // Errores
    std::ofstream out_errors("output/errores.txt");
    if (out_errors.is_open()) {
        for (const auto& e : output.errors) {
            out_errors << "Error Léxico: " << e.lexeme << " en L:" << e.line << " C:" << e.col << "\n";
        }
    }

    json_output = output.to_json();
}

static void on_script_message(WebKitUserContentManager* manager,
                               WebKitJavascriptResult* js_result,
                               gpointer user_data) {
    (void)manager;

    JSCValue* value = webkit_javascript_result_get_js_value(js_result);
    if (!jsc_value_is_string(value)) return;

    gchar* source_code_raw = jsc_value_to_string(value);
    std::string source_code(source_code_raw);
    g_free(source_code_raw);

    std::string json;
    execute_analysis_and_export(source_code, json);

    std::string script = "showResults('" + escape_for_js(json) + "')";

    WebKitWebView* webview = WEBKIT_WEB_VIEW(user_data);
    webkit_web_view_evaluate_javascript(webview, script.c_str(), -1,
                                        nullptr, nullptr, nullptr, nullptr);
}

int main(int argc, char* argv[]) {
    // Modo CLI estricto para revisión automática del profesor
    if (argc == 2) {
        std::string source_code = read_file(argv[1]);
        if (source_code.empty()) return 1;
        
        std::string dummy_json;
        execute_analysis_and_export(source_code, dummy_json);
        std::cout << "Análisis léxico completado exitosamente en modo CLI." << std::endl;
        std::cout << "Resultados guardados en la carpeta output/" << std::endl;
        return 0;
    }

    // Modo GUI Normal
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "LexLP Studio");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    WebKitUserContentManager* content_manager = webkit_user_content_manager_new();
    webkit_user_content_manager_register_script_message_handler(content_manager, "ipc");

    GtkWidget* webview = webkit_web_view_new_with_user_content_manager(content_manager);

    g_signal_connect(content_manager, "script-message-received::ipc",
                     G_CALLBACK(on_script_message), webview);

    std::string html_content = read_file("ui/index.html");
    if (!html_content.empty()) {
        webkit_web_view_load_html(WEBKIT_WEB_VIEW(webview), html_content.c_str(), nullptr);
    }

    gtk_container_add(GTK_CONTAINER(window), webview);
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
