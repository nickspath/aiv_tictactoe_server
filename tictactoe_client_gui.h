#include <raylib.h>


typedef struct Button {
    Rectangle bounds;
    int state;
    bool action;
    Color color;
    float roundness;
    int segments;
    char *text;
} button_s;

void DrawButton(Rectangle *r, Color c, float roundness, int segments, char *text);

typedef struct TextInput {
    Rectangle bounds;
    Color color;
} textInput_s;

void DrawTextInput (Rectangle *r, Color c, char *text);