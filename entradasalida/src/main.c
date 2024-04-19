#include <clientsIO.h>

int main(int argc, char* argv[]) {
    int err = clientToMEM();
    //int err = clientToKer();
    return err;
}
