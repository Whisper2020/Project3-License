#include "Server.h"

using namespace std;

int main() {
	Server S("20200"); //listen:20201
	S.Run();
	while (1);
	return 0;
}