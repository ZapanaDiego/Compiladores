// Funciones con parámetros, retorno y llamada con func.
int suma(int a, int b) {
    int r;
    r = a + b;
    return r;
}

void main() {
    int x;
    x = func suma(2, 3);
    println(x);
    return x;
}
