#include <stdint.h>

typedef struct Response {
    char playField[9];
    char serverAddress[15];
    char serverPort[5];
    int turn;
} response_s;

int InitSock(response_s *response);

int Join(char *name_i);

int CreateRoom();

int Challenge(uint32_t roomN);