#include <string>
#include <thread>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <thread>
#include <mutex>
#include <cstdio>
#include <cstring>
#include <vector>
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
    SOCKET mSock = INVALID_SOCKET, lSock = INVALID_SOCKET;
    int AddrSize = sizeof(sockaddr_in);
public:
    Server_Socket();
    ~Server_Socket();
    SOCKET passiveTCP(const char* service, int qlen);
    SOCKET passiveUDP(const char* service);
    void Disconnect();
    int SendUDP(PBYTE buf, int len);
    int RecvUDP(PBYTE buf, int len, int type = 0);
protected:
    struct sockaddr_in ClientAddr, AskAddr;
};

struct Cli_Info {
    SOCKADDR_IN Cli_Addr, Ask_Addr;
    int license;
};
typedef Cli_Info INFO, * PINFO;
class Server: Server_Socket {
public:
    void Run();
    Server(const char* port);
private:
    bool running = false;
    std::thread *tPoll;
    std::mutex mtx;
    std::vector <INFO> Cli_Cluster;
    void startPolling();
    int saveState()const;
    int loadState();
    friend void pollThread(Server* p);
};