/*
This is an implementation of REDIS database for study purposes only. Made following this book:
https://build-your-own.org/redis/03_hello_cs

Roque Santos - 2024-10-24
*/

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_PORT 1234

// This #pragma doesn't work with g++ compiler (was using this before changing from VSCode to Rider
// to circumvent this, the option "-lwsock32" was added on compiler options
#pragma comment(lib, "Ws2_32.lib")

static void do_something(int conn) {

    // read buffer - "request"
    char rbuf[64] = {};

    // read() -> linux
    // recv() -> windows
    SSIZE_T n = recv(conn, rbuf, sizeof(rbuf) - 1, 0);

    if (n < 0) {
        std::cout << "recv() error";
        return;
    }

    std::cout << "client says: " << rbuf << std::endl;

    // write buffer - "response"
    char wbuf[] = "world";
    send(conn, wbuf, strlen(wbuf), 0);
}

static int32_t one_request(int conn)
{
    return 0;
}

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
    // val: The value as arbitrary bytes we want to set as a c++ string, 1 meaning true in this case
    const char* val = "1";
    setsockopt(socketProc, SOL_SOCKET, SO_REUSEADDR, val, sizeof(val));

    // Binding the server to an ipv4 address
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;           // says address is ipv4 format
    addr.sin_port = ntohs(DEFAULT_PORT); // sets the addr port
    addr.sin_addr.s_addr = ntohl(0);     // sets the address ip as 0.0.0.0
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

    std::cout << "Server listening on " << DEFAULT_PORT << std::endl;

    // Server loop
    while (true) {
        // accept connections
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);
        int connProc = accept(socketProc, (struct sockaddr*)&client_addr, &addrlen);

        if (connProc < 0) {
            continue; // error
        }

        // do_something(connProc);

		while (true) {
			int32_t err = one_request(connProc);
		
			if (err) {
				break;
			}
		}

        // close()       -> linux impl
        // closesocket() -> windows impl
        closesocket(connProc);
    }

    WSACleanup();
    return 0;
}