#include <ctime>
#include <iostream>

using namespace std;
typedef unsigned long long ull;
namespace RSA {
	const ull mod = 998244359987710471ULL;
	const int e = 65537;
	ull mul(ull x, ull y) {
		ull ret = 0;
		while (y) {
			if (y & 1)
				ret = (ret + x) % mod;
			x = (x + x) % mod;
			y >>= 1;
		}
		return ret;
	}
	ull qpow(ull x, ull n) {
		ull ret = 1;
		while (n) {
			if (n & 1)
				ret = mul(ret,x);
			x = mul(x,x);
			n >>= 1;
		}
		return ret;
	}

	time_t RSAdecrypt(ull m) {
		return (time_t) qpow(m, e);
	}
	ull gentoken(time_t t) { //use in server, secret.
		std::cout<<"Generate token:"<<t<<std::endl;
		return qpow(t,78519762354634753ull); //this number is secret.
	}
};
signed main() {
	ull token = RSA::gentoken(time(NULL));
	//Server: send token to Client
	time_t m = RSA::RSAdecrypt(token);
	//Client: recv and decrypt
	cout<<"Decrypt: "<<m<<endl;
	if (time(NULL) - m < 5) //generated in 5s
		cout<<"Check OK"<<endl;
	else
		cout<<"Forgery."<<endl;
	return 0;
}