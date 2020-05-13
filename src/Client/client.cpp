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
    SOCKADDR_IN sin;
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
}
int Client::getLicense() {
    char str[] = "Helo!\n";
    Send((PBYTE)str, strlen(str) + 1);
    if (Receive((PBYTE)str, 10)>0)
        cout<<"Receive:"<<str<<endl;
    else {
        cout << "Network Error." << endl;
    }
    return 0;
}