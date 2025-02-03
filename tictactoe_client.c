#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include "tictactoe_client.h"

typedef struct SocketStuff {
    SOCKET s;
    WSADATA WSAData;
    struct sockaddr_in serverAddr, clientAddr;
} socketStuff_s;

socketStuff_s socketStuff;

void InitSock() {
    int iResult = 0;
    WSADATA WSAData;
    iResult = WSAStartup(MAKEWORD(2, 2), &WSAData);
    if (iResult != 0) {
        printf("Error at WSAStartup(): %d\n", WSAGetLastError());
    }
    socketStuff.WSAData = WSAData;
}

int StartSock(response_s *response, const char* address) {
    int iResult = 0;
    
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        return -1;
    }

    struct sockaddr_in serverAddr;
    InetPton(AF_INET, address, &serverAddr.sin_addr); //set address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(3621);  // set port

    struct sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(5621);
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    iResult = bind(s, (SOCKADDR *)&clientAddr, sizeof(clientAddr));
    if (iResult != 0) {
        printf("Bind failed with error: %d\n", WSAGetLastError());
        return -1;
    }

    unsigned long nonBlocking = 1;
    if (ioctlsocket(s, FIONBIO, &nonBlocking) != 0) {
        printf("Error setting non blocking mode: %d\n", WSAGetLastError());
        return -1;
    }

    int recvBufferSize = 1024;
    if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *)&recvBufferSize, sizeof(recvBufferSize)) != 0) {
        printf("Error setting receive buffer size: %d\n", WSAGetLastError());
        return -1;
    }

    socketStuff.s = s;
    socketStuff.serverAddr = serverAddr;
    socketStuff.clientAddr = clientAddr;

    InetNtop(AF_INET, &serverAddr.sin_addr, response->serverAddress, sizeof(response->serverAddress));  //fill response address
    sprintf(response->serverPort, "%lu", ntohl(serverAddr.sin_port));                                   //fill response port
    response->roomsCounter = 0;
    for (int i = 0; i < 9; i++) {
        response->playField[i] = ' ';
    }
    
    
    return 0;
}

char *GetLocalIPAddress() {
    char hostname[1024];
    struct addrinfo hints, *res;
    char *ip_address = malloc(sizeof(char) * INET_ADDRSTRLEN);
    struct hostent *host_info;
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        fprintf(stderr, "Get hostname failed: %d\n", WSAGetLastError());
        WSACleanup();
        return NULL;
    }
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(hostname, NULL, &hints, &res) != 0) {
        fprintf(stderr, "GetAddrInfoW failed: %d\n", WSAGetLastError());
        WSACleanup();
        return NULL;
    }

    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        void *addr;

        struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
        addr = &(ipv4->sin_addr);

        InetNtop(AF_INET, addr, ip_address, INET_ADDRSTRLEN);

        if (strstr(ip_address, "192.168") != NULL){
            return ip_address;
        }
    }

    // Free the linked list
    freeaddrinfo(res);

    return NULL;
}

int HandleNetwork(response_s *response) {    
    char recvBuf[1024];
    int recvBufLen = sizeof(recvBuf);
    int bytesReceived = 0;
    int fromLen = sizeof(socketStuff.serverAddr);

    bytesReceived = recvfrom(socketStuff.s, recvBuf, recvBufLen, 0, (SOCKADDR *)&socketStuff.serverAddr, &fromLen);
    if (bytesReceived == SOCKET_ERROR) {
        switch (WSAGetLastError()) {
            case WSAEWOULDBLOCK:
                return 0;
                break;            
            default:
                printf("Recvfrom failed with error: %d\n", WSAGetLastError());
                return -1;
                break;
        }
    } else if (bytesReceived == 12) {
        printf("Received %d bytes\n", bytesReceived);
        int values[3];
        for (size_t i = 0; i < 3; i++)
            memcpy(&values[i], recvBuf + sizeof(int) * i, sizeof(int));
        if (values[1] == 5) {   //room announce
            response->roomsCounter++;
            response->roomIDs = (int *)malloc(sizeof(int) * response->roomsCounter);
            response->roomIDs[response->roomsCounter - 1] = values[2];
        }
    } else if (bytesReceived == 8) {
        printf("Received %d bytes\n", bytesReceived);
        int values[2];
        for (size_t i = 0; i < 2; i++)
            memcpy(&values[i], recvBuf + sizeof(int) * i, sizeof(int));
        if (values[1] == 6) {   //game started
            response->scene = 1;
        }
    } else if (bytesReceived == (sizeof(int) * 2 + sizeof(char) * 9)) {
        printf("Received %d bytes\n", bytesReceived);
        int values[2];
        char playField[9];
        memcpy(&values[0], recvBuf, sizeof(int));
        memcpy(&values[1], recvBuf + sizeof(int), sizeof(int));
        for (size_t i = 0; i < 9; i++) {
            memcpy(&playField[i], recvBuf + sizeof(int) * 2 + sizeof(char) * i, sizeof(char));
            response->playField[i] = playField[i];
            printf("playfield[i] %c\n", response->playField[i]);
        }
        if (values[1] == 7) {   // ANNOUNCE PLAYFIELD   //useless check lol

        }
    }
    
    return 0;
}

int Join(char *name_i) {
    char pack[2 * 4 + 21];
    int values[] = {0, 0};
    for (int i = 0; i < 2; i++)
        memcpy(pack + i * 4, &values[i], sizeof(int)); // copy each integer
    memcpy(pack + sizeof(int) * 2, name_i, strlen(name_i)); //can't do sizeof(char*) only sizeof(char[]), use strlen(char*) instead
    sendto(socketStuff.s, pack, sizeof(int) * 2 + sizeof(char) * 20, 0, (SOCKADDR *)&socketStuff.serverAddr, sizeof(socketStuff.serverAddr));

    return 0;
}

int CreateRoom() {
    char pack[2 * 4];
    int values[] = {0, 4};
    memcpy(pack, &values[0], sizeof(int));
    memcpy(pack + sizeof(int), &values[1], sizeof(int));
    sendto(socketStuff.s, pack, sizeof(int) * 2, 0, (SOCKADDR *)&socketStuff.serverAddr, sizeof(socketStuff.serverAddr));

    return 0;
}

int Challenge(int roomN) {
    char pack[3 * 4];
    int values[] = {0, 1, roomN};
    memcpy(pack, &values[0], sizeof(int));
    memcpy(pack + sizeof(int), &values[1], sizeof(int));
    memcpy(pack + sizeof(int) * 2, &values[2], sizeof(int));
    sendto(socketStuff.s, pack, sizeof(int) * 3, 0, (SOCKADDR *)&socketStuff.serverAddr, sizeof(socketStuff.serverAddr));

    return 0;
}

int Move(int cellIndex) {
    char pack[3 * 4];
    int values[] = {0, 2, cellIndex};
    memcpy(pack, &values[0], sizeof(int));
    memcpy(pack + sizeof(int), &values[1], sizeof(int));
    memcpy(pack + sizeof(int) * 2, &values[2], sizeof(int));
    sendto(socketStuff.s, pack, sizeof(int) * 3, 0, (SOCKADDR *)&socketStuff.serverAddr, sizeof(socketStuff.serverAddr));

    return 0;
}

void CloseAndExit() {
    closesocket(socketStuff.s);
    WSACleanup();
}

// FREE ALL THE MALLOC

//send to only server ip, receive from any (it will just be server since it knows from send address)
//use select or poll instead of recvfrom always