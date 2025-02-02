#include "tictactoe_server.h"
#include "dictionary.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

// clang.exe -o tictactoe_server.exe .\tictactoe_server.c -lws2_32

#define COMMAND_JOIN 0
#define COMMAND_CHALLENGE 1
#define COMMAND_MOVE 2
#define COMMAND_QUIT 3
#define COMMAND_CREATE_ROOM 4
#define COMMAND_ANNOUNCE_ROOM 5

int RoomIsDoorOpen(room_s *room) {
    if (room->challenger.address != 0)
        return 0;
    else
        return -1;
}

int RoomHasStarted(room_s *room){
    for (size_t i = 0; i < 9; i++) {
        if (room->playField[i].address != 0) {
            return 0;
        }
    }
    return -1;
}

char* PrintSymbol(room_s *room, int cellN){
    player_s player = room->playField[cellN];
    if (player.playerType == None)
        return " ";
    else if (player.playerType == Owner)
        return "X";
    else if (player.playerType == Challenger)
        return "O";
    else
        return "?";
}

void RoomPrintPlayField(room_s *room){
    printf("|%s|%s|%s|\n", PrintSymbol(room, 0), PrintSymbol(room, 1), PrintSymbol(room, 2));
    printf("|%s|%s|%s|\n", PrintSymbol(room, 3), PrintSymbol(room, 4), PrintSymbol(room, 5));
    printf("|%s|%s|%s|", PrintSymbol(room, 6), PrintSymbol(room, 7), PrintSymbol(room, 8));
}

void RoomReset(room_s *room){
    room->challenger.playerType = None;
    for (int i = 0; i < 9; i++)
        room->playField[i].playerType = None;
    room->turn = room->owner;
    room->winner.playerType = None;
}

enum playerType RoomCheckHorizontal(room_s *room, int row){
    for (int col = 0; col < 3; col++) {
        if (room->playField[row * 3 + col].playerType == None)
            return None;
    }
    enum playerType player = room->playField[row * 3].playerType;
    if (room->playField[row * 3 + 1].playerType != player)
        return None;
    if (room->playField[row * 3 + 2].playerType != player)
        return None;
    return player;
}

enum playerType RoomCheckVertical(room_s *room, int col){
    for (int row = 0; row < 3; row++) {
        if (room->playField[row * 3 + col].playerType == None)
            return None;
    }
    enum playerType player = room->playField[col].playerType;
    if (room->playField[3 + col].playerType != player)
        return None;
    if (room->playField[6 + col].playerType != player)
        return None;
    return player;
}

enum playerType RoomCheckDiagonalLeft(room_s *room){
    for (int cell = 0; cell <= 8; cell+= 2)
        if (room->playField[cell].playerType == None)
            return None;
    enum playerType player = room->playField[0].playerType;
    if (room->playField[4].playerType != player)
        return None;
    if (room->playField[8].playerType != player)
        return None;
    return player;
}

enum playerType RoomCheckDiagonalRight(room_s *room){
    for (int cell = 2; cell <= 8; cell+= 2)
        if (room->playField[cell].playerType == None)
            return None;
    enum playerType player = room->playField[2].playerType;
    if (room->playField[4].playerType != player)
        return None;
    if (room->playField[6].playerType != player)
        return None;
    return player;
}

enum playerType RoomCheckVictory(room_s *room){
    for (int row = 0; row < 3; row++) {
        enum playerType winner = RoomCheckHorizontal(room, row);
        if (winner != None)
            return winner;
    }
    for (int col = 0; col < 3; col++)
    {
        enum playerType winner  = RoomCheckVertical(room, col);
        if (winner != None)
            return winner;
    }
    enum playerType winner = RoomCheckDiagonalLeft(room);
    if (winner != None){
        return winner;
    }
    return RoomCheckDiagonalRight(room);
}

int RoomMove(room_s *room, enum playerType player, int cellN){
    if (cellN < 0 || cellN > 8)
        return -1;
    if (room->playField[cellN].playerType != None)
        return -1;
    if (room->winner.playerType != None)
        return -1;
    if (room->challenger.playerType == None)
        return -1;
    if (player != room->owner.playerType && player != room->challenger.playerType)
        return -1;
    if (player != room->turn.playerType)
        return -1;
    room->playField[cellN].playerType = player;
    room->winner.playerType = RoomCheckVictory(room);
    if (room->turn.playerType == room->owner.playerType)
        room->turn.playerType = room->challenger.playerType;
    else
        room->turn.playerType = room->owner.playerType;
    return 0;
}

void ServerInitialize(server_s *server){
    server->roomCounter = 100;

    server->players = *DictInit(server->roomCounter);
    server->rooms = *DictInit(server->roomCounter);
    // maybe check for mem alloc integrity
}

int ServerStart(server_s *server, char address_p[], int port_p){
    int result;
    
    // Initialize Winsock
    result = WSAStartup(MAKEWORD(2, 2), &server->wsaData);    //makeword(2, 2) creates a word for required parameter WORD versionRequired
    if (result != 0){
        printf("WSAStartup failed: %d\n", result);
        return -1;
    }

    // Create socket
    server->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server->socket == INVALID_SOCKET){
        printf("Socket creation failed: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }
    // timeout

    // Configure Server Address
    server->serverAddr.sin_family = AF_INET;
    server->serverAddr.sin_port = htons(port_p);    // convert host to network byte order
    if (InetPton(AF_INET, address_p, &server->serverAddr.sin_addr) != 1) {
        printf("Invalid address or address not supported\n");
        return -1;
    }

    // Bind The Socket
    result = bind(server->socket, (struct sockaddr*)&server->serverAddr, sizeof(server->serverAddr));
    if (result == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(server->socket);
        WSACleanup();
        return -1;
    }

    printf("Server ready: waiting for packets...\n");

    return 0;
}

void ServerKick(server_s *server, char *sender) {
    player_s *badPlayer = (player_s *)DictGet(&server->players, sender, strlen(sender));
    if (badPlayer->room) {
        if (&badPlayer->room->owner == badPlayer) {
            printf("server room destroy\n");
            ServerRoomDestroy(server, badPlayer->room);
        }
        else {
            printf("dict remove\n");
            DictRemove(&server->players, sender, sizeof(sender));
        }
    }
}

void ServerRoomDestroy(server_s *server, room_s *room){
    char roomID_str[16];
    snprintf(roomID_str, sizeof(roomID_str), "%d", room->roomID);   // convert int to arr of char
    DictRemove(&server->rooms, roomID_str, sizeof(room->roomID));
    room->owner.room = NULL;
    if (room->challenger.playerType != None) {
        room->challenger.playerType = None;
    }
    printf("Room %d destroyed\n", room->roomID);
}

int ServerRoomRemovePlayer(server_s *server, char *sender) {
    player_s *player = DictGet(&server->players, sender, strlen(sender));
    if (!player->room) {
        DictRemove(&server->players, sender, sizeof(sender));
        printf("Player %s removed\n", player->name);
        return -1;
    }
    if (player == &player->room->challenger) {
        RoomReset(player->room);
        DictRemove(&server->players, sender, sizeof(sender));
        printf("Player %s removed\n", player->name);
        return -1;
    }
    ServerRoomDestroy(server, player->room);
    DictRemove(&server->players, sender, sizeof(sender));
    printf("Player %s removed\n", player->name);
    return 0;
}

int ServerTick(server_s *server){
    #define BUFFER_SIZE 64
    socklen_t addr_len = sizeof(server->serverAddr);
    char buffer[BUFFER_SIZE];

    int receivedBytes = recvfrom(server->socket, buffer, BUFFER_SIZE, 0,
                                    (struct sockaddr *)&server->serverAddr, &addr_len);
    if (receivedBytes < 0) {
        printf("Recvfrom error\n");
        return -1;
    }

    char sender[16]; //ipv4 length
    InetNtop(AF_INET, &server->serverAddr.sin_addr, sender, 16);    //converts address in sockaddr_in to sender
    const int port = ntohs(server->serverAddr.sin_port);
    
    printf("Sender address: %s, port: %d\n", sender, port);

    if (receivedBytes < 8) {
        printf("Invalid packet size: %d\n", receivedBytes);
        return -1;
    }

    int rid, command;
    memcpy(&rid, buffer, sizeof(int));
    memcpy(&command, buffer + sizeof(int), sizeof(int));

    if (command == COMMAND_JOIN && receivedBytes == 28) {
        InetPton(AF_INET, sender, &server->serverAddr.sin_addr);    // sockaddr_in addr to string
        if (DictGet(&server->players, sender, strlen(sender))) {
            printf("%s has already joined!\n", sender);
            ServerKick(server, sender);
            return -1;
        }
        //initialize player
        player_s *player = malloc(sizeof(player_s));
        char name[21];
        memcpy(name, buffer + sizeof(int) * 2, sizeof(char) * 20);
        name[20] = '\0';
        strcpy_s(player->name, sizeof(name), name);
        player->room = (room_s *)malloc(sizeof(room_s));
        player->room->roomID = 0;
        player->lastPacketTs = time(NULL);
        DictAdd(&server->players, sender, strlen(sender), player);
        printf("Player %s joined from %s [%d players on server]\n", player->name, // use dictfind instead?
        sender, DictCount(&server->players));
        return 0;
    }
    else if (command == COMMAND_CREATE_ROOM) {
        if (!DictGet(&server->players, sender, strlen(sender))) {
            printf("Unknown player %s\n", sender);
            return -1;
        }
        player_s *player = (player_s *)DictGet(&server->players, sender, strlen(sender));
        if (player->room->roomID != 0) {
            printf("Player %s (%s) already has a room\n", sender, player->name);
            return -1;
        }
        player->room = (room_s *)malloc(sizeof(room_s));
        player->room->roomID = server->roomCounter;
        player->room->owner = *player;
        RoomReset(player->room);

        char *roomCounter_buf = malloc((sizeof(char) * 11) + 1);
        sprintf(roomCounter_buf, "%u", server->roomCounter);    // int to string for dict key

        DictAdd(&server->rooms, roomCounter_buf, strlen(roomCounter_buf), player->room);
        printf("Room %s for player %s (%s) created\n", roomCounter_buf, sender, player->name);
        server->roomCounter++;
        player->lastPacketTs = time(NULL);
        return 0;
    }
    else if (command == COMMAND_CHALLENGE && receivedBytes == 12) {
        if (!DictGet(&server->players, sender, strlen(sender))) {
            printf("Unknown player %s\n", sender);
            return -1;
        }
        player_s *player = (player_s *)DictGet(&server->players, sender, strlen(sender));
        // if (player->room->roomID != 0) {                                                     //RE ENABLE
        //     printf("Player %s (%s) already in a room\n", sender, player->name);
        //     return -1;
        // }
        uint32_t roomCounter_int = 0;
        memcpy(&roomCounter_int, buffer + sizeof(uint32_t) * 2, sizeof(uint32_t));

        char *roomCounter_buf = malloc((sizeof(char) * 11) + 1);
        sprintf(roomCounter_buf, "%u", roomCounter_int);
        if (!DictGet(&server->rooms, roomCounter_buf, strlen(roomCounter_buf))) {
            printf("Unknown room %s\n", roomCounter_buf);
            return -1;
        }
        printf("after!!!!\n");
        room_s *room = (room_s *)DictGet(&server->rooms, roomCounter_buf, strlen(roomCounter_buf));
        if (!RoomIsDoorOpen(room)) {
            printf("Room %s is closed!\n", roomCounter_buf);
            return -1;
        }
        room->challenger = *player;
        player->room = room;
        player->lastPacketTs = time(NULL);
        printf("Game on room %s started!\n", roomCounter_buf);
        return 0;
    }
    else if (command == COMMAND_MOVE && receivedBytes == 12) {
        if (!DictGet(&server->players, sender, strlen(sender))) {
            printf("Uknown player %s\n", sender);
            return -1;
        }
        player_s *player = DictGet(&server->players, sender, strlen(sender));
        if (!player->room) {
            printf("Player %s (%s) is not in a room\n", sender, player->name);
            return -1;
        }
        int cell;
        memcpy(&cell, &buffer + 8, 4);
        if (!RoomMove(player->room, player->playerType, cell)) {  // fix room move
            printf("Player %s did an invalid move\n", player->name);
            return -1;
        }
        player->lastPacketTs = time(NULL);
        RoomPrintPlayField(player->room);
        if (player->room->winner.playerType != None) {
            printf("Player %s did WON!\n", player->room->winner.name);
            RoomReset(player->room);
            return 0;
        }
    }
    else if (command == COMMAND_QUIT) {
        if (!DictGet(&server->players, sender, strlen(sender))) {
            printf("Unknown player %s\n", sender);
            return -1;
        }
        ServerRoomRemovePlayer(server, sender);
        return 0;
    }
    else {
        printf("Unknown command from %s\n", sender);
        return -1;
    }
    printf("Boh\n");
    return -1;
}

void ServerAnnounces(server_s *server){
    room_s **rooms;
    int roomsCount = 0;
    for (size_t i = 0; i < server->rooms.hashmapSize; i++) {
        dictNode_s *node = server->rooms.nodes[i];
        while (node) {
            dictNode_s *nextNode = node->next;

            room_s *room = (room_s *)node->value;
            if (RoomIsDoorOpen(room)) {
                room_s **temp = calloc(roomsCount++, sizeof(room_s));
                if (!temp) {
                    printf("Memory allocation failed\n");
                    free(rooms);
                }
                rooms = temp;
                rooms[roomsCount++] = room;
            }

            node = nextNode;
        }
    }
    for (int i = 0; i < server->players.hashmapSize; i++) {
        dictNode_s *node = server->players.nodes[i];
        int count = 0;
        while (node) {
            dictNode_s *nextNode = node->next;

            player_s *player = (player_s *)node->value;
            if (player->room) {
                break;
            }
            for (int i = 0; i < roomsCount; i++) {
                printf("Announcing room %d to player %s\n", 
                    ((room_s *)node->value)->roomID, player->name);
                char packet[3 * 4];
                int values[] = {0, COMMAND_ANNOUNCE_ROOM, ((room_s *)node->value)->roomID};
                for (int i = 0; i < 3; i++)
                    memcpy(packet, &values[i] + sizeof(int) * i, sizeof(int));

                sendto(server->socket, packet, sizeof(int) * 3, 0,
                    (SOCKADDR *)&server->serverAddr, sizeof(server->serverAddr));
            }

            node = nextNode;
            printf("%p\n", node);
            count++;
        }
    }
}

int ServerCheckDeadPeers(server_s *server){
    time_t now = time(NULL);
    player_s **deadPlayers;
    int deadPlayersCount = 0;
    for (size_t i = 0; i < server->players.hashmapSize; i++) {
        dictNode_s *node = server->players.nodes[i];
        while (node) {
            dictNode_s *nextNode = node->next;

            player_s *player = DictGet(&server->players, node->key, sizeof(node->key));
            if (now - ((player_s *)node->value)->lastPacketTs > 30) {
                player_s **temp = calloc(deadPlayersCount++, sizeof(player_s));
                if (!temp) {
                    printf("Memory allocation in check dead peers failed\n");
                    free(temp);
                    return -1;
                }
                temp = deadPlayers;
            }

            node = nextNode;
        }
    }

    for (size_t i = 0; i < deadPlayersCount; i++) {
        printf("Removing %s for inactivity...\n", deadPlayers[i]->name);
        ServerRoomRemovePlayer(server, deadPlayers[i]->name);
    }

    return 0;
}

void ServerRun(server_s *server){
    while (1) {
        ServerTick(server);
        ServerAnnounces(server);
        ServerCheckDeadPeers(server);
    }
}

void ServerClose(server_s *server) {
    DictDealloc(&server->players);
    DictDealloc(&server->rooms);
    WSACleanup();
}

int main() {
    server_s server;

    ServerInitialize(&server);
    ServerStart(&server, "192.168.1.200", 3621);
    ServerRun(&server);
    ServerClose(&server);

    return 0;
}