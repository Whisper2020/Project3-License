#include "Server.h"

using namespace std;
bool operator==(const sockaddr_in& lhs, const sockaddr_in& rhs) {
	return (memcmp(&lhs, &rhs, sizeof(SOCKADDR_IN)) == 0);
}
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
	Disconnect();
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
	running = true;
	startPolling();
	while (1) {
		RecvUDP((PBYTE)str/* 修改为需要的数据结构*/, 6/*修改为对应的sizeof()*/);
		cerr << "Get connect from [" << inet_ntoa(ClientAddr.sin_addr) << ": " << ntohs(ClientAddr.sin_port) << "] "<<str<<endl;
		//判断是否予以注册
		if (strcmp(str, "Helo!") == 0) {//judge(str, ClientAddr)
			Cli_Cluster.push_back(ClientAddr); //加入轮询队列
			//Addrunning(str, ClientAddr);
			SendUDP((PBYTE)"OK", strlen("OK"));
		}
		else {
			Cli_Cluster.erase(find(Cli_Cluster.begin(), Cli_Cluster.end(), ClientAddr));//从队列移除
			//Removerunning(str, ClientAddr);
			SendUDP((PBYTE)"BYE", strlen("BYE"));
		}
		
	}
	return;
}

Server::Server(const char* port)
{
	passiveUDP(port);
	cout << "Listening on " << port << endl;
}

void pollThread(Server* p) {
	char str[50];
	const char service[] = "20201", transport[] = "UDP";
	int type;
	PSERVENT pse;
	PPROTOENT ppe;
	SOCKADDR_IN sin;
	u_short port, pre;
	SOCKET s = INVALID_SOCKET;
	int AddrSize = sizeof(sockaddr_in);
	if (pse = getservbyname(service, transport))
		port = pse->s_port;
	else if ((port = htons((USHORT)atoi(service))) == 0) {
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

	s = socket(AF_INET, type, ppe->p_proto);
	if (s == INVALID_SOCKET) {
		cerr << "can't create socket: " << GetLastError() << endl;
		exit(-1);
	}
	while (p->running) {
		cerr << "Checking " << p->Cli_Cluster.size() << " Client.." << endl;
		for (size_t i = 0; i < p->Cli_Cluster.size(); ++i) {
			sin = p->Cli_Cluster.at(i);
			pre = sin.sin_port;
			sin.sin_port = port;
			sendto(s, (char *)"Are you running?", 18, 0, (PSOCKADDR)&sin, AddrSize);
			if (recvfrom(s, str, 50, 0, (PSOCKADDR)&sin, &AddrSize) <= 0) {//掉线
				cout << "Disconnect with " << inet_ntoa(sin.sin_addr) << ": " << ntohs(pre) << endl;
				//修改记录已掉线的客户端的状态
				//ChangeState(inet_ntoa(sin.sin_addr), ntohs(pre));
				p->Cli_Cluster.erase(p->Cli_Cluster.begin() + i);
			}
			else {
				cout << "Client " << i << " is OK." << endl;
			}
		}
		//Sleep for 10s
		Sleep(10000);
	}
}
void Server::startPolling()
{
	tPoll = new thread(pollThread, this);
	cerr << "Start polling thread." << endl;
}
