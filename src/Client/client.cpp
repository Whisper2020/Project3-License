#include "client.h"
#pragma once
Client_Socket::~Client_Socket() {
    Disconnect();
    WSACleanup();
}
Client_Socket::Client_Socket() {
    WORD wVersion = MAKEWORD(2, 1);
    WSAStartup(wVersion, &WSAData);
}
void Client_Socket::Disconnect() {
    if (s != INVALID_SOCKET)
        closesocket(s);
    s = INVALID_SOCKET;
}
SOCKET Client_Socket::connectTCP(const char* host, const char* service) {
    return connectSock(host, service, "TCP");
}
SOCKET Client_Socket::connectUDP(const char* host, const char* service) {
    return connectSock(host, service, "UDP");
}
SOCKET Client_Socket::connectSock(const char* host, const char* service, const char* transport) {
    int type;
    PHOSTENT phe;
    PSERVENT pse;
    PPROTOENT ppe;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    if (pse = getservbyname(service, transport))
        sin.sin_port = pse->s_port;
    else if ((sin.sin_port = htons((USHORT)atoi(service))) == 0) {
        cerr << "can't get \"" << service << "\" service entry." << endl;
        exit(-1);
    }
    if (phe = gethostbyname(host))
        memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
    else if ((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE) {
        cerr << "can't get \"" << host << "\" host entry." << endl;
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
    if (connect(s, (PSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR) {
        cerr << "can't connect to " << host << ":" <<service<<", ErrorNo = "<<GetLastError()<< endl;
        exit(-1);
    }
    return s;
}
int Client_Socket::Receive(PBYTE buf, int len) {
    return recv(s, (char *)buf, len, 0);
}
int Client_Socket::Send(PBYTE buf, int len) {
    return send(s, (char*)buf, len, 0);
}
Client::Client(const char* hostip, const char* port){
    connectUDP(hostip, port);
    running = true;
}

int Client::getLicense() {
    char str[] = "Helo!";
    Send((PBYTE)str/* 修改为需要的数据结构*/, 6/*修改为对应的sizeof()*/);
    if (Receive((PBYTE)str, 3) > 0) {
        cout << "Receive:" << str << endl;
        startAnswer();
    }
    else {
        cout << "Network Error." << endl;
    }
    return 0;
}
int Client::retLicense()
{
    char str[] = "Bye!";
    Send((PBYTE)str/* 修改为需要的数据结构*/, 5/*修改为对应的sizeof()*/);
    if (Receive((PBYTE)str, 4) > 0)
        cout << "Receive:" << str << endl;
    else {
        cout << "Network Error. Can't give back license." << endl;
        return -1;
    }
    running = false;
    Disconnect();
    return 0;
}
void answerThread(Client *p) {
    char str[50];
    const char service[] = "20200", transport[] = "UDP";
    int type;
    int AddrSize = sizeof(sockaddr_in);
    PSERVENT pse;
    PPROTOENT ppe;
    SOCKADDR_IN sin = p->sin;
    SOCKET mSock = INVALID_SOCKET;
    if (pse = getservbyname(service, transport))
        sin.sin_port = pse->s_port;
    else if ((sin.sin_port = htons((USHORT)atoi(service) ^ 1)) == 0) {
        cerr << "can't get \"" << service << "\"^1 service entry." << endl;
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
    if (sendto(mSock, "Ans", 4, 0, (PSOCKADDR)&sin, AddrSize) <= 0) {
        cerr << "Send Ans error." << endl;
        exit(-1);
    }
    while (p->running && recvfrom(mSock, (char*)str/* 修改为需要的数据结构*/, 50/* 修改为需要的长度*/, 0, (PSOCKADDR)&sin, &AddrSize) > 0) {
        cout << "Receive from server: " << str << endl;
        sendto(mSock, (char*)"I'm running.\n"/* 修改为需要的数据结构*/, 15/* 修改为需要的长度*/, 0, (PSOCKADDR)&sin, AddrSize);
    }
    closesocket(mSock);
    if (p->running)
        cerr << "Disconnect from License Server." << endl;
}
void Client::startAnswer()
{
    cerr << "Start answer thread." << endl;
    tAns = new thread(answerThread, this);
}