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
}
