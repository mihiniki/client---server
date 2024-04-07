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

#pragma comment(lib, "ws2_32.lib")

//std::mutex mutex;

class Server {
private:
    struct sockaddr_in serverAddr {};
    SOCKET socketServer;
public:
    void connectWithClient(int socketClient) {


        int cntRows;
        int cntCols;

        std::vector<std::thread> threadVector;
        char buffer[1024] = { '\0' };

        int readBytes = recv(socketClient, buffer, sizeof(buffer), 0);

        if (readBytes < 0) {
            perror("read socket problem");
            _close(socketClient);
            return;
        }
        buffer[readBytes] = '\0';

        int fillNum;
        int cntThreads;

        sscanf(buffer, "%d, %d, %d, %d", &cntRows, &cntCols, &cntThreads, &fillNum);
        std::vector<std::vector<int>> table(cntRows, std::vector<int>(cntCols, 0));

        int threadWork = cntRows / cntThreads;

        for (int i = 0; i < cntThreads; i++) {
            int startFill = i;
            int endFill;
            if (i == cntThreads - 1) {
                endFill = cntRows;
            }
            else {
                endFill = startFill + threadWork;
            }

            threadVector.emplace_back(&Server::fillTable, this, std::ref(table), fillNum, startFill, endFill);
        }

        for (int i = 0; i < cntThreads; i++) {
            threadVector[i].join();
        }

        std::string infoForTable;
        for (int i = 0; i < cntRows; i++) {
            for (int j = 0; j < cntCols; j++) {
                infoForTable += std::to_string(table[i][j]) + " ";
            }
            infoForTable += "\n";
        }

        send(socketClient, infoForTable.c_str(), infoForTable.length(), 0);
        //  _close(socketClient);
    }

    void fillTable(std::vector<std::vector<int>>& table, int number, int startRow, int endRow) {
        for (int i = startRow; i < endRow; i++) {
            int size = table[i].size();
            for (int j = 0; j < size; j++) {
                table[i][j] = number;
            }
        }
    }

    void serverStart() {
        if (listen(socketServer, 3) < 0) {

            std::cout << "listening failed";
        }
        std::cout << "Waiting:";
        //  for (;;) {
        socklen_t serverAddrLen = sizeof(serverAddr);
        int newSocket = accept(socketServer, (sockaddr*)&serverAddr, &serverAddrLen);
        if (newSocket == -1) {
            perror("accept");
            //continue;
        }
        std::thread newThread = std::thread(&Server::connectWithClient, this, newSocket);
        newThread.join();
        //newThread.detach(); // ���������� ������ �����, �� �� �� ������� ��������� �����
        // }

    }

    Server() {


        socketServer = socket(AF_INET, SOCK_STREAM, 0);
        if (socketServer == INVALID_SOCKET)
        {
            std::cerr << "Error creating socket\n";
            exit(EXIT_FAILURE);
            //WSACleanup();
            return;
        }
        //std::cout << "here";
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(8080);
        if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
            std::cerr << "Invalid address\n";
            closesocket(socketServer);
            WSACleanup();
            exit(EXIT_FAILURE);
        }


        //std::cout << "here";
        if (bind(socketServer, (const sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("bind");
            exit(EXIT_FAILURE);
        }
        // std::cout << "here";
    }

    ~Server() {
        // std::cout << "here";
        closesocket(socketServer);
        WSACleanup();
        //std::cout << "here";

    }
};

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock\n";
        return -1;
    }
    //std::cout << "here";
    Server server;
    server.serverStart();
    return 0;
}
