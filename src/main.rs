mod lexer;
mod token;

use lexer::Lexer;
use tao::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::WindowBuilder,
};
use wry::{WebViewBuilder, WebViewBuilderExtWindows};

fn main() -> wry::Result<()> {
    let event_loop = EventLoop::new();

    let window = WindowBuilder::new()
        .with_title("LexLP - Fase 1: Lectura + Números")
        .with_inner_size(tao::dpi::LogicalSize::new(1000.0, 700.0))
        .build(&event_loop)
        .unwrap();

    let html_content = include_str!("../ui/index.html");

    let _webview = WebViewBuilder::new()
        .with_html(html_content)
        .with_ipc_handler(move |webview, msg| {
            // msg contains the source code sent from JavaScript
            let source_code = msg.body().to_string();

            // Execute lexical analysis in Rust
            let output = Lexer::analyze(&source_code);

            // Convert result to JSON
            let json = serde_json::to_string(&output).unwrap_or_else(|_| "{}".to_string());

            // Send result back to JavaScript
            let script = format!("showResults('{}')", json.replace('\\', "\\\\").replace('\'', "\\'"));
            webview.evaluate_script(&script).unwrap();
        })
        .build(&window)?;

    event_loop.run(move |event, _, control_flow| {
        *control_flow = ControlFlow::Wait;

        if let Event::WindowEvent {
            event: WindowEvent::CloseRequested,
            ..
        } = event
        {
            *control_flow = ControlFlow::Exit;
        }
    });
}
