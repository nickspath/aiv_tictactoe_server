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
    int address;
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
    WSADATA wsaData;
    SOCKET socket;
    struct sockaddr_in serverAddr;
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

int ServerStart(server_s *server, char address_p[], int port_p);

void ServerKick(server_s *server, char *sender);

void ServerRoomDestroy(server_s *server, room_s* room);

int ServerRoomRemovePlayer(server_s *server, char *sender);