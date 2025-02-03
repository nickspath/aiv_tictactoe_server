#include <stdio.h>
#include <stdlib.h>
#include "tictactoe_client_gui.h"
#include "tictactoe_client.h"
#include <string.h>

#define MAX_INPUT_CHARS_NAME 20

int main(int argc, char const *argv[])
{
    response_s response;
    InitSock();
    const char *address;
    if (argc > 1)
        address = argv[1];
    else
        address = GetLocalIPAddress();
    printf("%s\n", address);
    StartSock(&response, address);

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Tictactoe client window");
    SetTargetFPS(60);


    textInput_s nameTextInput = {
        .bounds = { 150, 125, 200, 75 },
        .color = GRAY,
        .fontSize = 18
    };
    char nameText[20] = "";
    int nameLetterCount = 0;
    bool nameMouseOnText = false;

    textInput_s roomTextInput = {
        .bounds = { 200, 225, 100, 75 },
        .color = GRAY,
        .fontSize = 18
    };
    char roomText[10] = "";
    int roomLetterCount = 0;
    bool roomMouseOnText = false;

    button_s joinButton = {
        .bounds = {450, 75, 175, 75},
        .state = 0,
        .action = false,
        .text = "Join",
        .color = GRAY,
        .roundness = 0.15f,
        .segments = 1,
        .fontSize = 18
    };
    button_s createButton = {
        .bounds = {450, 175, 175, 75},
        .state = 0,
        .action = false,
        .text = "Create",
        .color = GRAY,
        .roundness = 0.15f,
        .segments = 1,
        .fontSize = 18
    };
    button_s challengeButton = {
        .bounds = {450, 275, 175, 75},
        .state = 0,
        .action = false,
        .text = "Challenge",
        .color = GRAY,
        .roundness = 0.15f,
        .segments = 1,
        .fontSize = 18
    };
    button_s buttons[] = {joinButton, createButton, challengeButton};
    button_s playfieldButtons[9];
    for (int i = 0; i < 9; i++) {
        // char c[2];
        // snprintf(c, sizeof(char) * 2, "%c", response.playField[i]);
        Rectangle r = { 300 + 75 * (i % 3), 100 + 75 * (i / 3), 64, 64 };
        playfieldButtons[i].bounds = r;
        playfieldButtons[i].text = " ";
        //free(c);
        playfieldButtons[i].action = false;
        playfieldButtons[i].state = 0;
        playfieldButtons[i].color = GRAY;
        playfieldButtons[i].roundness = 0.15f;
        playfieldButtons[i].segments = 1;
        playfieldButtons[i].fontSize = 32;
    }
    

    Vector2 mousePoint = {0, 0};

    char ip[24] = "";
    snprintf(ip, sizeof(ip), "IP: %s:%s", response.serverAddress, response.serverPort);

    while (!WindowShouldClose()) {
        mousePoint = GetMousePosition();
        if (response.scene == 0) {
            for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
                buttons[i].action = false;
                if (CheckCollisionPointRec(mousePoint, buttons[i].bounds)) {
                    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                        buttons[i].state = 2;   // press
                    else
                        buttons[i].state = 1;   // hover

                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                        buttons[i].action = true;
                }
                else
                    buttons[i].state = 0;   // nothing
            }
            if (buttons[0].action)
                Join(nameText);
            if (buttons[1].action) 
                CreateRoom();
            if (buttons[2].action) {
                uint32_t t = strtoul(roomText, NULL, 10);
                Challenge(t);
            }        

            if (CheckCollisionPointRec(GetMousePosition(), nameTextInput.bounds))
                nameMouseOnText = true;
            else
                nameMouseOnText = false;
            if (nameMouseOnText) {
                SetMouseCursor(MOUSE_CURSOR_IBEAM);
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 32) && (key <= 125) && (nameLetterCount < sizeof(nameText))) {
                        nameText[nameLetterCount] = (char)key;
                        nameText[nameLetterCount + 1] = '\0';
                        nameLetterCount++;
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    nameLetterCount--;
                    if (nameLetterCount < 0)
                        nameLetterCount = 0;
                    nameText[nameLetterCount] = '\0';
                }
            }
            else
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
            //
            if (CheckCollisionPointRec(GetMousePosition(), roomTextInput.bounds))
                roomMouseOnText = true;
            else
                roomMouseOnText = false;
            if (roomMouseOnText) {
                SetMouseCursor(MOUSE_CURSOR_IBEAM);
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= 48) && (key <= 57) && (roomLetterCount < sizeof(roomText))) {
                        roomText[roomLetterCount] = (char)key;
                        roomText[roomLetterCount + 1] = '\0';
                        roomLetterCount++;
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE)) {
                    roomLetterCount--;
                    if (roomLetterCount < 0)
                        roomLetterCount = 0;
                    roomText[roomLetterCount] = '\0';
                }
            }
            else
                SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }
        else if (response.scene == 1) {
            //GAME SCENE PLAYFIELD BUTTONS LOGIC
            for (int i = 0; i < 9; i++) {
                playfieldButtons[i].action = false;
                if (CheckCollisionPointRec(mousePoint, playfieldButtons[i].bounds)) {
                    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                        playfieldButtons[i].state = 2;
                    else
                        playfieldButtons[i].state = 1;
                    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                        playfieldButtons[i].action = true;
                }
                else
                    playfieldButtons[i].state = 0;
            }
            for (int i = 0; i < 9; i++)
                if (playfieldButtons[i].action)
                    Move(i);
            if (IsKeyReleased(KEY_F)) {
                response.scene = 0;
                //do other reset stuff
            }
        }
        
        // DRAW
        HandleNetwork(&response);
        BeginDrawing();
            ClearBackground(RAYWHITE);
            if (response.scene == 0) {
                DrawText("name: ", nameTextInput.bounds.x - 70, nameTextInput.bounds.y + 25, 18, GRAY);
                DrawText("roomid: ", roomTextInput.bounds.x - 70, roomTextInput.bounds.y + 25, 18, GRAY);
                DrawText(ip, 25, 400, 28, GRAY);
                for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
                    if (buttons[i].state == 2) {
                        Color darkerColor = {buttons[i].color.r + 35, buttons[i].color.g + 35, buttons[i].color.b + 35, 255};
                        DrawButton(&buttons[i].bounds, darkerColor, buttons[i].roundness, buttons[i].segments, buttons[i].text, buttons[i].fontSize);
                    } else {
                        DrawButton(&buttons[i].bounds, buttons[i].color, buttons[i].roundness, buttons[i].segments, buttons[i].text, buttons[i].fontSize);
                    }
                }
                DrawTextInput(&nameTextInput.bounds, nameTextInput.color, nameText, nameTextInput.fontSize);
                DrawTextInput(&roomTextInput.bounds, roomTextInput.color, roomText, roomTextInput.fontSize);
                //available rooms
                for (size_t i = 0; i < response.roomsCounter; i++) {
                    char text[10];
                    snprintf(text, sizeof(text), "%d", response.roomIDs[i]);
                    Rectangle r = {665, 100 + 150 * i, 100, 40};
                    DrawRectangle(r.x, r.y, r.width, r.height, LIGHTGRAY);
                    int textX = r.x + r.width / 2 - MeasureText(text, 16) / 2;
                    int textY = r.y + r.height / 2 - GetFontDefault().baseSize / 2;
                    DrawText(text, textX, textY, 22, GRAY);
                }
            } else if (response.scene == 1) {
                for (int i = 0; i < 9; i++) {
                    char c[2];
                    c[0] = response.playField[i];
                    c[1] = '\0';
                    Color cc;
                    Color dc = {playfieldButtons[i].color.r + 35, playfieldButtons[i].color.g + 35, playfieldButtons[i].color.b + 35, 255};
                    if (playfieldButtons[i].state == 2) {
                        cc = playfieldButtons[i].color;
                    } else {
                        cc = dc;
                    }
                    DrawButton(
                        &playfieldButtons[i].bounds, 
                        cc, 
                        playfieldButtons[i].roundness, 
                        playfieldButtons[i].segments,
                        c,
                        playfieldButtons[i].fontSize
                    );
                }
            }
        EndDrawing();
    }
    CloseWindow();


    return 0;
}

void DrawButton(Rectangle *r, Color c, float roundness, int segments, char *text, int fontSize) {
    DrawRectangleRounded(*r, roundness, segments, c);
    Color c2 = { .r = c.r + 0x40, .g = c.g + 0x40, .b = c.b + 0x40, .a = 255 };
    int textX = r->x + r->width / 2 - MeasureText(text, fontSize) / 2;
    int textY = r->y + r->height / 2 - GetFontDefault().baseSize;
    DrawText(text, textX, textY, fontSize, c2);
}

void DrawTextInput(Rectangle *r, Color c, char *text, int fontSize) {
    DrawRectangleRec(*r, c);
    Color darkerColor = { .r = c.r + 0x40, .g = c.g + 0x40, .b = c.b + 0x40, .a = 255 };
    DrawRectangleLines((int)r->x, (int)r->y, (int)r->width, (int)r->height, darkerColor);
    int textX = r->x + r->width / 2 - MeasureText(text, fontSize) / 2;
    int textY = r->y + r->height / 2 - GetFontDefault().baseSize / 2;
    DrawText(text, textX, textY, fontSize, darkerColor);
}
