#include <cstdio>
#include <io.h>
#include <string>
#include <string.h>
#include <vector>
#include "WinSock2.h"
#include <iostream>
#include <thread>
#pragma once
#pragma comment(lib, "Ws2_32.lib")
using namespace std;


class  Client_Socket{
private:
    const int recvsize = 0, sendsize = 0;
    WSADATA WSAData;
    SOCKET s = INVALID_SOCKET;
    SOCKET connectSock(const char* host, const char* service, const char* transport);
public:
    Client_Socket();
    ~Client_Socket();
    SOCKET connectTCP(const char* host, const char* service);
    SOCKET connectUDP(const char* host, const char* service);
    int Receive(PBYTE buf, const int len);
    int Send(PBYTE buf, const int len);
    void Disconnect();
protected:
    SOCKADDR_IN sin;
};

class Client: protected Client_Socket {
public:
    int getLicense();
    int retLicense();
    Client(const char* hostip, const char* port);
private:
    bool running = false;
    thread *tAns = NULL;
    void startAnswer();
    friend void answerThread(Client* p);
};