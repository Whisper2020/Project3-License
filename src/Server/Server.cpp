#include "Server.h"

using namespace std;

Server_Socket::Server_Socket() {
#ifndef Linux
	WORD wVersion = MAKEWORD(2, 1);
	if (WSAStartup(wVersion, &WSAData) != 0) {
		cerr << "WSAStartup failen.\n" << endl;
		exit(-1);
	}
#endif
}
Server_Socket::~Server_Socket() {
	closesocket(mSock);
#ifndef Linux
	WSACleanup();
#endif
}
SOCKET Server_Socket::passiveUDP(const char* service) {
	return passiveSock(service, "UDP", 0);
}

void Server_Socket::Disconnect()
{
	if (mSock != INVALID_SOCKET)
		closesocket(mSock);
	mSock = INVALID_SOCKET;
}

int Server_Socket::SendUDP(PBYTE buf, int len)
{
	return sendto(mSock, (char*)buf, len, 0, (PSOCKADDR)&ClientAddr, sizeof(ClientAddr));
}

int Server_Socket::RecvUDP(PBYTE buf, int len)
{
	return recvfrom(mSock, (char*)buf, len, 0, (PSOCKADDR) & ClientAddr, &ClientAddrSize);
}

SOCKET Server_Socket::passiveTCP(const char* service, int qlen) {
	return passiveSock(service, "TCP", qlen);
}
SOCKET Server_Socket::passiveSock(const char* service, const char* transport, int qlen) {
	int type;
	PSERVENT pse;
	PPROTOENT ppe;
	SOCKADDR_IN sin;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	if (pse = getservbyname(service, transport))
		sin.sin_port = pse->s_port;
	else if ((sin.sin_port = htons((USHORT)atoi(service))) == 0) {
		cerr << "can't get \"" << service << "\" service entry." << endl;
		exit(-1);
	}
	if ((ppe = getprotobyname(transport)) == 0) {
		cerr << "can't get \"" << transport << "\" protocol entry." << endl;
		exit(-1);
	}
	if (_stricmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;
	mSock = socket(AF_INET, type, ppe->p_proto);
	if (mSock == INVALID_SOCKET) {
		cerr << "can't create socket: " << GetLastError() << endl;
		exit(-1);
	}
	if (bind(mSock, (PSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR) {
		cerr << "can't bind to service(port): " << service << ", ErrorNo = " << GetLastError() << endl;
		exit(-1);
	}
	if (type == SOCK_STREAM && listen(mSock, qlen) == SOCKET_ERROR) {
		cerr << "can't listen on port(service): " << service << ", ErrorNo = " << GetLastError() << endl;
		exit(-1);
	}
	return mSock;
}

void Server::Run()
{
	char str[50];
	while (1) {
		RecvUDP((PBYTE)str, 100);
		cerr << "Get connect from " << inet_ntoa(ClientAddr.sin_addr) << ": " << ntohs(ClientAddr.sin_port) << endl;
		//judge();
		SendUDP((PBYTE)"OK.\n", strlen("OK.\n") + 1);
	}
	return;
}

Server::Server(const char* port)
{
	cout << "Listening on " << port << endl;
	passiveUDP(port);
}
