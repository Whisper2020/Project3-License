#include "client.h"

int main() {
    Client C("127.0.0.1", "20200");
    C.getLicense();
    return 0;
}