#include "dictionary.h"
#include <WinSock2.h>
#include <stdint.h>

typedef struct Player player_s;
typedef struct Room room_s;

enum playerType {
    None,
    Owner,
    Challenger
};

typedef struct Player {
    enum playerType playerType;
    char name[20 + 1];
    room_s *room;
    float lastPacketTs;
    char address[16];
} player_s;

typedef struct Room {
    int roomID;
    player_s owner;
    player_s challenger;
    player_s playField[9];
    player_s turn;
    player_s winner;
} room_s;

typedef struct Server {
    struct dict players;    // dictionary of players in the server
    struct dict rooms;
    uint32_t roomCounter;
    size_t playersCounter;
    WSADATA wsaData;
    SOCKET socket;
    struct sockaddr_in sockAddr;
} server_s;

char* PrintSymbol(room_s *room, int cellN);

void RoomPrintPlayField(room_s *room);

enum playerType RoomCheckHorizontal(room_s *room, int row);

enum playerType RoomCheckVertical(room_s *room, int col);

enum playerType RoomCheckDiagonalLeft(room_s *room);

enum playerType RoomCheckDiagonalRight(room_s *room);

enum playerType RoomCheckVictory(room_s *room);

int RoomMove(room_s *room, enum playerType player, int cellN);

void ServerInitialize(server_s *server);

int ServerStart(server_s *server, const char address_p[], const int port_p);

void ServerAnnounces(server_s *server);

void BroadCastRoomStart(server_s *server, room_s *room);

void BroadCastPlayField(server_s *server, room_s *room, char playField[9]);

void ServerKick(server_s *server, char *sender);

void ServerRoomDestroy(server_s *server, room_s* room);

int ServerRoomRemovePlayer(server_s *server, char *sender);

char *GetLocalIPAddress();