/*
This is an implementation of REDIS database for study purposes only. Made following this book:
https://build-your-own.org/redis/03_hello_cs

Roque Santos - 2024-10-24
*/

#include <assert.h>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#define DEFAULT_PORT 1234

// This #pragma doesn't work with g++ compiler (was using this before changing from VSCode to Rider
// to circumvent this, the option "-lwsock32" was added on compiler options
#pragma comment(lib, "Ws2_32.lib")

const size_t k_max_msg = 4096;

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

    std::cout << "client says: " << rbuf << "\n";

    // write buffer - "response"
    char wbuf[] = "world";
    send(conn, wbuf, strlen(wbuf), 0);
}

static int32_t read_full(int conn, char* buf, size_t n)
{
    while (n > 0)
    {
        SSIZE_T rv = recv(conn, buf, n, 0);

        if (rv <= 0)
        {
            return -1;
        }
        assert(static_cast<size_t>(rv) <= n);

        n -= static_cast<size_t>(rv);
        buf += rv;
    }

    return 0;
}

static int32_t write_all(int conn, char* buf, size_t n)
{
    while (n > 0)
    {
        SSIZE_T rv = send(conn, buf, n, 0);

        if (rv <= 0)
        {
            return -1;
        }

        assert(static_cast<size_t>(rv) <= n);
        n -= static_cast<size_t>(rv);
        buf += rv;
    }
    
    return 0;
}

static int32_t one_request(int conn)
{
    // 4 bytes header
    // (32bit int with buf size) + (max buf len) + (1bit buf end)
    char rbuf[4 + k_max_msg + 1];
    errno = 0;

    int32_t err = read_full(conn, rbuf, 4);
    if (err)
    {
        if (errno == 0)
        {
            std::cout << "EOF\n";
        }
        else
        {
            std::cout << "Error handling request " << conn << ": read() error\n";
        }

        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4);

    if (len > k_max_msg)
    {
        std::cout << "Message too long\n";
        return -1;
    }

    // request body
    err = read_full(conn, &rbuf[4], len);
    if (err)
    {
        std::cout << "read() error\n";
        return err;
    }

    // do something
    rbuf[4 + len] = '\0';
    std::cout << "client says: " << &rbuf[4] << "\n";

    // reply using same protocol
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = static_cast<uint32_t>(strlen(reply));

    // 1. copies the reply length to the first 4 bytes of the write buffer
    //    the first 4 bytes of the stream is the content length (as a 32 bits unsigned integer) to the peer
    memcpy(wbuf, &len, 4);

    // 2. copies the reply content to the buffer starting at the 4th position
    //    (offsetting the first 4 bytes that are reserved for the content length)
    memcpy(&wbuf[4], reply, len); 

    return write_all(conn, wbuf, 4 + len);
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