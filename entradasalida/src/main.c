/*#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>

int main(int argc, char* argv[]) {
    decir_hola("una Interfaz de Entrada/Salida");
    return 0;
}*/
#include <clientToKer.h>
#include <clientToMem.h>

int main(int argc, char* argv[]) {
    int resultMEM = clientToMEM();
    return 0;

    int resultKER = clientToKer();
    return 0;
}
