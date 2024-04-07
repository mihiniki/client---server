#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <thread>
#include <cstring>
#include <winsock2.h>
#include <winsock.h>
#include <sys/types.h>
#include <io.h>
#include <string>
#include <cstdio>
#include <mutex>
#include <WS2tcpip.h>


#pragma comment(lib, "ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
std::mutex mutex;

class Client {
private:
    struct sockaddr_in serverAddr {};
    SOCKET socketClient;

public:
    Client() {
        socketClient = socket(AF_INET, SOCK_STREAM, 0);
        if (socketClient == -1) {
            perror("socket");
            exit(EXIT_FAILURE);
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(8080);

        if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid address\n";
            closesocket(socketClient);
            WSACleanup();
            exit(EXIT_FAILURE);
        }
    }

    ~Client() {
        closesocket(socketClient);
    }

    void connectServer() {

        int res = ::connect(socketClient, (const sockaddr*)&serverAddr, sizeof(serverAddr));
        if (res == SOCKET_ERROR) {
            int error_code = WSAGetLastError();
            std::cerr << "Connection error: " << error_code << std::endl;
            exit(EXIT_FAILURE);
            // Можете да използвате конкретни кодове на грешки за диагностициране
            // и решаване на проблема
        }

        // std::cout << "here";
    }

    void sendRequest(int cntRows, int cntCols, int cntThreads, int fillNumber) {
        char bufferTalk[1024] = { '\0' };
        char bufferRes[1024] = { '\0' };
        snprintf(bufferTalk, sizeof(bufferTalk), "%d, %d, %d, %d", cntRows, cntCols, cntThreads, fillNumber);

        int res = ::send(socketClient, bufferTalk, strlen(bufferTalk), 0);
        if (res == -1) {
            perror("send");
            exit(EXIT_FAILURE);
        }

        int readBytes = ::recv(socketClient, bufferRes, sizeof(bufferRes), 0);
        if (readBytes == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        bufferRes[readBytes] = '\0';
        printf("Answer: \n%s\n", bufferRes);
    }
};

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        WSACleanup();
        return 1;
    }

    Client client;

    client.connectServer();

    int cntRows, cntCols, cntThreads, fillNumber;
    std::cout << "Enter cntRows, cntCols, cntThreads and fillNumber: ";
    std::cin >> cntRows >> cntCols >> cntThreads >> fillNumber;

    client.sendRequest(cntRows, cntCols, cntThreads, fillNumber);

    WSACleanup();
    return 0;
}
