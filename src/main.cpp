#include "Lexer.h"
#include "Parser.h"
#include "SymbolTable.h"

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

static bool read_file(const std::string& path, std::string& content) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo: " << path << std::endl;
        return false;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
    return true;
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

struct AnalysisResult {
    LexerOutput lexer;
    ParseResult parser;
    bool outputs_ok = true;
};

static AnalysisResult execute_analysis_and_export(const std::string& source_code,
                                                  std::string& json_output) {
    SymbolTable symTable;
    LexerOutput output = Lexer::analyze(source_code, symTable);

    ParseResult parseResult;
    if (output.errors.empty()) {
        parseResult = Parser::parse(output.tokens);
    } else {
        // Evita errores sintácticos en cascada cuando el flujo léxico ya falló.
        parseResult.executed = false;
        parseResult.accepted = false;
    }

    bool outputs_ok = true;
    std::error_code ec;
    std::filesystem::create_directories("output", ec);
    if (ec) {
        std::cerr << "Error: No se pudo crear el directorio output/: "
                  << ec.message() << std::endl;
        outputs_ok = false;
    }

    // Archivo de Tokens (Formato exigido por la entrega)
    std::ofstream out_tokens("output/tokens.txt");
    if (!out_tokens.is_open()) {
        std::cerr << "Error: No se pudo escribir output/tokens.txt" << std::endl;
        outputs_ok = false;
    } else {
        bool firstToken = true;
        for (const auto& t : output.tokens) {
            if (!firstToken) {
                out_tokens << " ";
            }
            out_tokens << t.toString();
            firstToken = false;
        }
        out_tokens << "\n";
        if (!out_tokens.good()) {
            std::cerr << "Error al escribir output/tokens.txt" << std::endl;
            outputs_ok = false;
        }
    }

    // Tabla de Símbolos (Formato exigido por la entrega)
    if (!symTable.exportToFile("output/tabla_simbolos.txt")) {
        std::cerr << "Error: No se pudo escribir output/tabla_simbolos.txt"
                  << std::endl;
        outputs_ok = false;
    }

    // Errores léxicos
    std::ofstream out_errors("output/errores.txt");
    if (!out_errors.is_open()) {
        std::cerr << "Error: No se pudo escribir output/errores.txt" << std::endl;
        outputs_ok = false;
    } else {
        out_errors << "Línea\tColumna\tLexema\tResultado\n";
        for (const auto& e : output.errors) {
            out_errors << e.line << "\t" << e.col << "\t" << e.lexeme
                       << "\tERROR_LEXICO\n";
        }
        if (!out_errors.good()) {
            std::cerr << "Error al escribir output/errores.txt" << std::endl;
            outputs_ok = false;
        }
    }

    // Errores sintácticos
    std::ofstream out_syntax("output/errores_sintacticos.txt");
    if (!out_syntax.is_open()) {
        std::cerr << "Error: No se pudo escribir output/errores_sintacticos.txt"
                  << std::endl;
        outputs_ok = false;
    } else {
        out_syntax << "Línea\tColumna\tEncontrado\tEsperado\tMensaje\n";
        for (const auto& e : parseResult.errors) {
            out_syntax << e.line << "\t" << e.col << "\t" << e.found << "\t"
                       << e.expected << "\t" << e.message << "\n";
        }
        if (!out_syntax.good()) {
            std::cerr << "Error al escribir output/errores_sintacticos.txt"
                      << std::endl;
            outputs_ok = false;
        }
    }

    // Resultado global del análisis sintáctico
    std::ofstream out_result("output/resultado_sintactico.txt");
    if (!out_result.is_open()) {
        std::cerr << "Error: No se pudo escribir output/resultado_sintactico.txt"
                  << std::endl;
        outputs_ok = false;
    } else {
        if (!parseResult.executed) {
            out_result << "NO EJECUTADO\n";
        } else if (parseResult.accepted) {
            out_result << "ACEPTADO\n";
        } else {
            out_result << "RECHAZADO\n";
        }
        if (!out_result.good()) {
            std::cerr << "Error al escribir output/resultado_sintactico.txt"
                      << std::endl;
            outputs_ok = false;
        }
    }

    json_output = output.to_json();
    if (!json_output.empty() && json_output.back() == '}') {
        json_output.pop_back();
        json_output += ",\"syntax\":" + parseResult.to_json() + "}";
    }

    return AnalysisResult{std::move(output), std::move(parseResult), outputs_ok};
}

static void print_lexical_errors(const std::vector<LexerError>& errors) {
    for (const auto& e : errors) {
        std::cerr << "Línea " << e.line << ", columna " << e.col
                  << " (léxico): " << e.message << " ['" << e.lexeme << "']\n";
    }
}

static void print_syntax_errors(const std::vector<SyntaxError>& errors) {
    for (const auto& e : errors) {
        std::cerr << "Línea " << e.line << ", columna " << e.col
                  << " (sintáctico): " << e.message << "\n";
    }
}

static int run_cli(const std::string& path) {
    std::string source_code;
    if (!read_file(path, source_code)) {
        return 1;
    }

    std::string json;
    AnalysisResult result = execute_analysis_and_export(source_code, json);

    if (!result.lexer.errors.empty()) {
        print_lexical_errors(result.lexer.errors);
        std::cerr << "Análisis sintáctico no ejecutado: corrige los errores "
                     "léxicos primero."
                  << std::endl;
        return 1;
    }

    if (!result.parser.executed) {
        std::cerr << "Análisis sintáctico no ejecutado." << std::endl;
        return 1;
    }

    if (!result.parser.accepted) {
        print_syntax_errors(result.parser.errors);
        std::cout << "Resultado sintáctico: RECHAZADO" << std::endl;
    } else {
        std::cout << "Resultado sintáctico: ACEPTADO" << std::endl;
    }

    if (result.outputs_ok) {
        std::cout << "Resultados guardados en la carpeta output/" << std::endl;
    } else {
        std::cerr << "No se pudieron guardar todos los resultados en output/"
                  << std::endl;
    }

    return (result.parser.accepted && result.outputs_ok) ? 0 : 1;
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
                                        nullptr, nullptr, nullptr, nullptr,
                                        nullptr);
}

static void print_usage(const char* program) {
    std::cout << "Uso:\n"
              << "  " << program << "                 # Abre la interfaz gráfica\n"
              << "  " << program << " archivo.lp        # Analiza en modo CLI\n"
              << "  " << program << " --help            # Muestra esta ayuda\n";
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        std::string arg = argv[1];
        if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            return 0;
        }
        return run_cli(arg);
    }

    if (argc > 2) {
        std::cerr << "Error: demasiados argumentos." << std::endl;
        print_usage(argv[0]);
        return 2;
    }

    // Modo GUI Normal
    gtk_init(&argc, &argv);

    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "LexLP Studio");
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 700);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), nullptr);

    WebKitUserContentManager* content_manager = webkit_user_content_manager_new();
    webkit_user_content_manager_register_script_message_handler(content_manager,
                                                                "ipc");

    GtkWidget* webview =
        webkit_web_view_new_with_user_content_manager(content_manager);

    g_signal_connect(content_manager, "script-message-received::ipc",
                     G_CALLBACK(on_script_message), webview);

    std::string html_content;
    if (read_file("ui/index.html", html_content)) {
        gchar* current_dir = g_get_current_dir();
        std::string base_uri =
            "file://" + std::string(current_dir) + "/ui/";
        g_free(current_dir);

        webkit_web_view_load_html(WEBKIT_WEB_VIEW(webview),
                                  html_content.c_str(), base_uri.c_str());
        gtk_container_add(GTK_CONTAINER(window), webview);
    } else {
        GtkWidget* label = gtk_label_new(
            "No se encontró ui/index.html.\n"
            "Ejecuta el programa desde la raíz del repositorio.");
        gtk_container_add(GTK_CONTAINER(window), label);
    }

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
