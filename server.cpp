/*
This is an implementation of REDIS database for study purposes only. Made following this book:
https://build-your-own.org/redis/03_hello_cs

Roque Santos - 2024-10-24
*/

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

// #pragma comment(lib, "Ws2_32.lib")
// This #pragma doesnt work with g++ compiler, which is the one i'm using
// to circumvent this, the option "-lwsock32" was added on compiler options

int main() {

    // Initializes windows sockets
    WORD versionWanted = MAKEWORD(1, 1);
    WSADATA wsaData;
    WSAStartup(versionWanted, &wsaData);

    // Obtaining socket handle
    int socketProc = socket(AF_INET, SOCK_STREAM, 0);

    // Socket configurations
    // SOL_SOCKET: Says that we want to set a socket level configuration
    // SO_REUSEADDR: https://stackoverflow.com/questions/3229860/what-is-the-meaning-of-so-reuseaddr-setsockopt-option-linux/3233022#3233022
    // val: The value as arbritary bytes we want to set as an c++ string, 1 meaning true in this case
    const char* val = "1";
    setsockopt(socketProc, SOL_SOCKET, SO_REUSEADDR, val, sizeof(val));

    // Binding the server to an ipv4 address
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0); // address 0.0.0.0
    int statusCode = bind(socketProc, (const sockaddr*)&addr, sizeof(addr));

    if (statusCode) {
        std::cout << "Socket binding failed with code " << statusCode << std::endl;
        WSACleanup();
        return 1;
    }

    // Listen on address
    statusCode = listen(socketProc, SOMAXCONN);

    if (statusCode) {
        std::cout << "Socket listening failed with code " << statusCode << std::endl;
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;
}