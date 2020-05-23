#include "client.h"

int main() {
    Client C("1.28.250.189", "20200");
    //cout << "Input License:" << endl;
    //cin >> lic;
    C.getLicense();
    Sleep(5000);
    //while (1);
    C.retLicense();

    return 0;
}