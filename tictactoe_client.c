#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>

int main(int argc, char const *argv[]) {
    WSADATA WSAData;
    WSAStartup(MAKEWORD(2, 2), &WSAData);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    InetPton(AF_INET, "192.168.1.200", &serverAddr.sin_addr); //set address
    serverAddr.sin_port = htons(3621);  // set port
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    const unsigned int timeout = 1000;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(unsigned int));

    // join packet
    char pack[2 * 4 + 21];
    uint32_t values[] = {0, 0};
    char name[21] = "NicolaSpataro0000000";
    for (int i = 0; i < 2; i++)
        memcpy(pack + i * 4, &values[i], sizeof(uint32_t)); // copy each integer
    memcpy(pack + sizeof(int) * 2, name, sizeof(name));
    sendto(s, pack, sizeof(uint32_t) * 2 + sizeof(char) * 20, 0, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
    
    // create room packet
    char pack3[2 * 4];
    uint32_t values3[] = {0, 4};
    memcpy(pack3, &values3[0], sizeof(uint32_t));
    memcpy(pack3 + sizeof(uint32_t), &values3[1], sizeof(uint32_t));
    sendto(s, pack3, sizeof(uint32_t) * 2, 0, (SOCKADDR *)&serverAddr, sizeof(serverAddr));

    // challenge packet
    char pack2[3 * 4];
    long argv1 = strtol(argv[1], NULL, 10); //string to long
    uint32_t values2[] = {0, 1, argv1};
    memcpy(pack2, &values2[0], sizeof(int));
    memcpy(pack2 + sizeof(int), &values2[1], sizeof(int));
    memcpy(pack2 + sizeof(int) * 2, &values2[2], sizeof(int));
    sendto(s, pack2, sizeof(uint32_t) * 3, 0, (SOCKADDR *)&serverAddr, sizeof(serverAddr));

    WSACleanup();
    
    return 0;
}

