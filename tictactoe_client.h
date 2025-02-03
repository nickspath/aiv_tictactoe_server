#include <stdint.h>

typedef struct Response {
    char playField[9];
    char serverAddress[15];
    char serverPort[5];
    int turn;
    int *roomIDs;
    int roomsCounter;
    int scene;  // 0 main, 1 game
} response_s;

int InitSock(response_s *response);

int Join(char *name_i);

int CreateRoom();

int Challenge(int roomN);

int Move(int cellIndex);

int HandleNetwork(response_s *response);

char *GetLocalIPAddress();

void CloseAndExit();