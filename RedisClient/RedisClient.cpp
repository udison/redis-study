#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

int main() {
    WORD versionWanted = MAKEWORD(1, 1);
    WSADATA wsaData;
    WSAStartup(versionWanted, &wsaData);

    int socketProc = socket(AF_INET, SOCK_STREAM, 0);
    if (socketProc < 0) {
        std::cout << "Failed to create socket process";
        WSACleanup();
        return 1;
    }

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
    
    int statusCode = connect(socketProc, (const struct sockaddr*)&addr, sizeof(addr));
    if (statusCode) {
        std::cout << "Failed to connect to " << addr.sin_addr.s_addr << ":" << 1234 << " (code: " << statusCode << ")" << std::endl;
        WSACleanup();
        return 1;
    }

    char msg[] = "hello";
    send(socketProc, msg, strlen(msg), 0);

    char rbuf[64] = {};
    SSIZE_T n = recv(socketProc, rbuf, sizeof(rbuf) - 1, 0);
    if (n < 0) {
        std::cout << "Failed to read buffer" << std::endl;
        WSACleanup();
        return 1;
    }

    std::cout << "server says: " << rbuf << std::endl;
    closesocket(socketProc);
}