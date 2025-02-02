#include <stdio.h>
#include <stdlib.h>
#include "tictactoe_client_gui.h"
#include "tictactoe_client.h"
#include <string.h>

#define MAX_INPUT_CHARS_NAME 20

int main(int argc, char const *argv[])
{
    response_s response;
    InitSock(&response);

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Tictactoe client window");
    SetTargetFPS(60);


    textInput_s nameTextInput = {
        .bounds = { 100, 100, 175, 75 },
        .color = GRAY,
    };
    char nameText[20] = "";
    int nameLetterCount = 0;
    bool nameMouseOnText = false;

    textInput_s roomTextInput = {
        .bounds = { 100, 200, 175, 75 },
        .color = GRAY,
    };
    char roomText[10] = "";
    int roomLetterCount = 0;
    bool roomMouseOnText = false;

    button_s joinButton = {
        .bounds = {350, 75, 175, 75},
        .state = 0,
        .action = false,
        .text = "Join",
        .color = GRAY,
        .roundness = 0.15f,
        .segments = 1
    };
    button_s createButton = {
        .bounds = {350, 175, 175, 75},
        .state = 0,
        .action = false,
        .text = "Create",
        .color = GRAY,
        .roundness = 0.15f,
        .segments = 1
    };
    button_s challengeButton = {
        .bounds = {350, 275, 175, 75},
        .state = 0,
        .action = false,
        .text = "Challenge",
        .color = GRAY,
        .roundness = 0.15f,
        .segments = 1
    };
    button_s buttons[] = {joinButton, createButton, challengeButton};

    Vector2 mousePoint = {0, 0};

    char ip[24] = "";
    snprintf(ip, sizeof(ip), "IP: %s:%s", response.serverAddress, response.serverPort);

    while (!WindowShouldClose()) {
        mousePoint = GetMousePosition();
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
        
        // DRAW
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText(ip, 25, 400, 28, GREEN);
            for (size_t i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
                DrawButton(&buttons[i].bounds, buttons[i].color, buttons[i].roundness, buttons[i].segments, buttons[i].text);
            }
            DrawTextInput(&nameTextInput.bounds, nameTextInput.color, nameText);
            DrawTextInput(&roomTextInput.bounds, roomTextInput.color, roomText);
        EndDrawing();
    }
    CloseWindow();

    return 0;
}

void DrawButton(Rectangle *r, Color c, float roundness, int segments, char *text) {
    DrawRectangleRounded(*r, roundness, segments, c);
    Color c2 = { .r = c.r + 0x40, .g = c.g + 0x40, .b = c.b + 0x40, .a = 255 };
    int textX = r->x + r->width / 2 - MeasureText(text, r->width / 10) / 2;
    int textY = r->y + r->height / 2 - GetFontDefault().baseSize / 2;
    DrawText(text, textX, textY, r->width / 10, c2);
}

void DrawTextInput(Rectangle *r, Color c, char *text) {
    DrawRectangleRec(*r, c);
    Color darkerColor = { .r = c.r + 0x40, .g = c.g + 0x40, .b = c.b + 0x40, .a = 255 };
    DrawRectangleLines((int)r->x, (int)r->y, (int)r->width, (int)r->height, darkerColor);
    DrawText(text, (int)r->x + 5, (int)r->y + 8, r->width / 10, darkerColor);
}
