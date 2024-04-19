#include <serverCPU.h>
#include <clientsCPU.h>

int main(int argc, char* argv[]) {
    int err;
    err = clientToMem();
    err = iniciar_servidor();
    return err;
}