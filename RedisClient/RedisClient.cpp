#include <cassert>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

const size_t k_max_msg = 4096;

// TODO: Move this to an external library, this is copied from RedisServer.cpp
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

// TODO: Move this to an external library, this is copied from RedisServer.cpp
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

static int32_t query(int conn, const char *text)
{
    uint32_t len = static_cast<uint32_t>(strlen(text));
    if (len > k_max_msg)
    {
        return -1;
    }

    char wbuf[4 + k_max_msg];
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], text, len);
    if (int32_t err = write_all(conn, wbuf, 4 + len))
    {
        return err;
    }

    // 4 bytes header
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
            std::cout << "Error handling query: read() error\n";
        }

        return err;
    }

    memcpy(&len, rbuf, 4);
    if (len > k_max_msg)
    {
        std::cout << "Message length is too large\n";
        return -1;
    }

    // reply body
    err = read_full(conn, &rbuf[4], len);
    if (err)
    {
        std::cout << "Error reading data";
        return err;
    }

    // do something
    rbuf[4 + len] = '\0';
    std::cout << &rbuf[4] << "\n";
    return 0;
}

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
        std::cout << "Failed to connect to " << addr.sin_addr.s_addr << ":" << 1234 << " (code: " << statusCode << ")\n";
        WSACleanup();
        return 1;
    }

    // Page 3 testing:
    // char msg[] = "hello";
    // send(socketProc, msg, strlen(msg), 0);
    //
    // char rbuf[64] = {};
    // SSIZE_T n = recv(socketProc, rbuf, sizeof(rbuf) - 1, 0);
    // if (n < 0) {
    //     std::cout << "Failed to read buffer" << std::endl;
    //     WSACleanup();
    //     return 1;
    // }
    // std::cout << "server says: " << rbuf << std::endl;

    // Multiple requests
    int32_t err = query(socketProc, "Hello World!");
    if (err)
    {
        // i'm fr using goto
        goto L_DONE;
    }

    err = query(socketProc, "Hello once again!");
    if (err)
    {
        // i'm fr using goto
        goto L_DONE;
    }

    err = query(socketProc, "Hello one more time!");
    if (err)
    {
        // i'm fr using goto
        goto L_DONE;
    }

L_DONE:
    closesocket(socketProc);
}