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
    println("Mensaje con \"comillas\" internas");

    // 5. Operadores contiguos y comparaciones
    int a = 10;
    int b = 20;
    if (a <= b && !(_contador != 0)) {
        a = a + + b;
    }

    // 6. Comentario de cierre
    return;
}
