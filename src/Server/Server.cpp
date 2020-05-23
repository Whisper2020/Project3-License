#include "Server.h"

using namespace std;
bool operator==(const INFO& lhs, const SOCKADDR_IN& rhs) {
	return (memcmp(&lhs.Cli_Addr, &rhs, sizeof(SOCKADDR_IN)) == 0);
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

int Server_Socket::RecvUDP(PBYTE buf, int len, int type)
{
	if (type == 0)
		return recvfrom(mSock, (char*)buf, len, 0, (PSOCKADDR) & ClientAddr, &AddrSize);
	else
		return recvfrom(lSock, (char*)buf, len, 0, (PSOCKADDR)&AskAddr, &AddrSize);
}

SOCKET Server_Socket::passiveTCP(const char* service, int qlen) {
	return passiveSock(service, "TCP", qlen);
}
SOCKET Server_Socket::passiveSock(const char* service, const char* transport, int qlen) {
	int type;
	PSERVENT pse;
	PPROTOENT ppe;
	SOCKADDR_IN sin, sin2;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	if (pse = getservbyname(service, transport))
		sin.sin_port = pse->s_port;
	else if ((sin.sin_port = htons((USHORT)atoi(service))) == 0) {
		cerr << "can't get \"" << service << "\" service entry." << endl;
		exit(-1);
	}
	sin2 = sin;
	if (pse = getservbyname(service, transport))
		sin2.sin_port = pse->s_port;
	else if ((sin2.sin_port = htons((USHORT)atoi(service) ^ 1)) == 0) {
		cerr << "can't get \"" << service << "\" ^1 service entry." << endl;
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
	lSock = socket(AF_INET, type, ppe->p_proto);
	if (mSock == INVALID_SOCKET || lSock == INVALID_SOCKET) {
		cerr << "can't create socket: " << GetLastError() << endl;
		exit(-1);
	}
	if (bind(mSock, (PSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR || bind(lSock, (PSOCKADDR)&sin2, sizeof(sin2)) == SOCKET_ERROR) {
		cerr << "can't bind to service(port): " << service << ", ErrorNo = " << GetLastError() << endl;
		exit(-1);
	}
	return mSock;
}

void Server::Run()
{
	PINFO pI;
	char str[50];
	running = true;
	startPolling();
	while (1) {
		pI = new INFO();
		RecvUDP((PBYTE)str/* 修改为需要的数据结构*/, 6/*修改为对应的sizeof()*/);
		cerr << "Get connect from [" << inet_ntoa(ClientAddr.sin_addr) << ": " << ntohs(ClientAddr.sin_port) << "] "<<str<<endl;
		//判断是否予以注册
		if (strcmp(str, "Helo!") == 0) {//judge(str, ClientAddr)
			pI->Cli_Addr = ClientAddr;
			SendUDP((PBYTE)"OK", 3);
			if (RecvUDP((PBYTE)str/* 修改为需要的数据结构*/, 6/*修改为对应的sizeof()*/, 1) > 0) {
				pI->Ask_Addr = AskAddr;
				cerr << "Ans thread is at: [" << inet_ntoa(AskAddr.sin_addr) << ": " << ntohs(AskAddr.sin_port) << "] " << str << endl;
			}
			else
				cerr << "P2P Error." << endl;
			mtx.lock();
			Cli_Cluster.push_back(*pI); //加入轮询队列
			saveState();
			mtx.unlock();
			//Addrunning(str, ClientAddr);
			
		}
		else {
			mtx.lock();
			Cli_Cluster.erase(find(Cli_Cluster.begin(), Cli_Cluster.end(), ClientAddr));//从队列移除
			mtx.unlock();
			//Removerunning(str, ClientAddr);
			SendUDP((PBYTE)"BYE", 4);
		}
		delete pI;
	}
	return;
}

Server::Server(const char* port)
{
	loadState();
	passiveUDP(port);
	cout << "Listening on " << port << endl;
}

void pollThread(Server* p) {
	char str[50];
	const char service[] = "20203", transport[] = "UDP";
	int type;
	PPROTOENT ppe;
	SOCKADDR_IN sin;
	u_short pre;
	SOCKET s = INVALID_SOCKET;
	TIMEVAL tv;
	int AddrSize = sizeof(sockaddr_in);
	if ((ppe = getprotobyname(transport)) == 0) {
		cerr << "can't get \"" << transport << "\" protocol entry." << endl;
		exit(-1);
	}
	if (_stricmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	s = socket(AF_INET, type, ppe->p_proto);
	tv.tv_sec = 1;
	tv.tv_usec = 500;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(TIMEVAL));
	if (s == INVALID_SOCKET) {
		cerr << "can't create socket: " << GetLastError() << endl;
		exit(-1);
	}
	while (p->running) {
		cerr << "Checking " << p->Cli_Cluster.size() << " Client.." << endl;
		p->mtx.lock();
		for (size_t i = 0; i < p->Cli_Cluster.size(); ++i) {
			sin = p->Cli_Cluster.at(i).Ask_Addr;
			pre = p->Cli_Cluster.at(i).Cli_Addr.sin_port;
			sendto(s, (char *)"Are you running?", 17, 0, (PSOCKADDR)&sin, AddrSize);
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
		p->saveState();
		p->mtx.unlock();
		//Sleep for 10s
		Sleep(10000);
	}
}
void Server::startPolling()
{
	tPoll = new thread(pollThread, this);
	cerr << "Start polling thread." << endl<<endl;
}

int Server::saveState() const
{
	if (Cli_Cluster.size() == 0) {
		if (_access("data.bin", 0) == 0) {
			if (remove("data.bin") == -1) {
				cerr << "Can;t remove temp file." << endl;
				return -1;
			}
		}
		return 0;
	}
	ofstream fout("data.bin", ios::binary);
	if (!fout) {
		cerr << "Create data file error." << endl;
		return -1;
	}
	for_each(Cli_Cluster.begin(), Cli_Cluster.end(), [&](INFO x) {fout.write((char*)&x, sizeof(INFO)); });
	cerr << "save state success" << endl << endl;
	fout.close();
	return 0;
}

int Server::loadState()
{
	ifstream fin("data.bin", ios::binary);
	if (fin) {
		INFO x;
		int cnt = 0;
		cerr << "Recovering..  ";
		while (fin.peek() != EOF) {
			fin.read((char*)&x, sizeof(INFO));
			Cli_Cluster.emplace_back(x);
			++cnt;
		}
		cerr << "Load " << cnt << " records." << endl;
		fin.close();
		if (remove("data.bin") == -1) {
			cerr << "Can;t remove temp file." << endl;
			return -1;
		}
	}
	return 0;
}
