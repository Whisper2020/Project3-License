#include "client.h"

int main() {
    Client C("127.0.0.1", "20200");
    //cout << "Input License:" << endl;
    //cin >> lic;
    C.getLicense();
    Sleep(5000);
    while (1);
    C.retLicense();

    return 0;
}