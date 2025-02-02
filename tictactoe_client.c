#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include "tictactoe_client.h"

typedef struct SocketStuff {
    SOCKET s;
    WSADATA WSAData;
    struct sockaddr_in serverAddr;
} socketStuff_s;

socketStuff_s socketStuff;

int InitSock(response_s *response) {

    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 2), &WSAData);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    InetPton(AF_INET, "192.168.1.200", &serverAddr.sin_addr); //set address
    serverAddr.sin_port = htons(3621);  // set port
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    const unsigned int timeout = 1000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(unsigned int));
    socketStuff.s = s;
    socketStuff.serverAddr = serverAddr;
    socketStuff.WSAData = WSAData;

    InetNtop(AF_INET, &serverAddr.sin_addr, response->serverAddress, sizeof(response->serverAddress));
    sprintf(response->serverPort, "%lu", ntohl(serverAddr.sin_port));
    
    return 0;
}

int Join(char *name_i) {
    char pack[2 * 4 + 21];
    uint32_t values[] = {0, 0};
    char name[21] = "NicolaSpataro0000000";
    for (int i = 0; i < 2; i++)
        memcpy(pack + i * 4, &values[i], sizeof(uint32_t)); // copy each integer
    memcpy(pack + sizeof(int) * 2, name_i, strlen(name_i)); //can't do sizeof(char*) only sizeof(char[]), use strlen(char*) instead
    sendto(socketStuff.s, pack, sizeof(uint32_t) * 2 + sizeof(char) * 20, 0, (SOCKADDR *)&socketStuff.serverAddr, sizeof(socketStuff.serverAddr));

    return 0;
}

int CreateRoom() {
    char pack3[2 * 4];
    uint32_t values3[] = {0, 4};
    memcpy(pack3, &values3[0], sizeof(uint32_t));
    memcpy(pack3 + sizeof(uint32_t), &values3[1], sizeof(uint32_t));
    sendto(socketStuff.s, pack3, sizeof(uint32_t) * 2, 0, (SOCKADDR *)&socketStuff.serverAddr, sizeof(socketStuff.serverAddr));

    return 0;
}

int Challenge(uint32_t roomN) {
    char pack2[3 * 4];
    uint32_t values2[] = {0, 1, roomN};
    memcpy(pack2, &values2[0], sizeof(uint32_t));
    memcpy(pack2 + sizeof(uint32_t), &values2[1], sizeof(uint32_t));
    memcpy(pack2 + sizeof(uint32_t) * 2, &values2[2], sizeof(uint32_t));
    sendto(socketStuff.s, pack2, sizeof(uint32_t) * 3, 0, (SOCKADDR *)&socketStuff.serverAddr, sizeof(socketStuff.serverAddr));

    return 0;
}

//receive rooms announcement
//name input in gui
//room input in gui
//available rooms in gui
//different screens
//move input in room screen
// RECEIVE SHIT