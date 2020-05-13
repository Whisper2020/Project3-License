#include <string>
#include <thread>
#include <iostream>
#include <thread>
#include <cstdio>
#include <cstring>
#include <Windows.h>
//#include <WinSock2.h>
#include <direct.h>
#include <io.h>
#define socklen_t int

#pragma once
#pragma comment(lib, "Ws2_32.lib")

class  Server_Socket {
private:
#ifndef Linux
    WSADATA WSAData;
#endif
    SOCKET passiveSock(const char* service, const char* transport, int qlen);
    SOCKET mSock = INVALID_SOCKET;
    int ClientAddrSize = sizeof(ClientAddr);
public:
    Server_Socket();
    ~Server_Socket();
    SOCKET passiveTCP(const char* service, int qlen);
    SOCKET passiveUDP(const char* service);
    void Disconnect();
    int SendUDP(PBYTE buf, int len);
    int RecvUDP(PBYTE buf, int len);
protected:
    struct sockaddr_in ClientAddr;
};

class Server: Server_Socket {
public:
    void Run();
    Server(const char* port);

};