// Control de flujo anidado.
void main() {
    int i;
    i = 0;
    while (i < 3) {
        if (i == 1) {
            println("uno");
        } else {
            println("otro");
        }
        i = i + 1;
    }
    return i;
}
