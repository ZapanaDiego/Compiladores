// Precedencia de operadores y expresiones.
void main() {
    int a;
    int b;
    int c;
    a = 1;
    b = 2;
    c = (a + b) * 2 - 3;
    if (a < b && !(c == 0) || a >= b) {
        c = -c + +a;
    }
    return c;
}
