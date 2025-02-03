#include <raylib.h>


typedef struct Button {
    Rectangle bounds;
    int state;
    bool action;
    Color color;
    float roundness;
    int segments;
    char *text;
    int fontSize;
} button_s;

void DrawButton(Rectangle *r, Color c, float roundness, int segments, char *text, int fontSize);

typedef struct TextInput {
    Rectangle bounds;
    Color color;
    int fontSize;
} textInput_s;

void DrawTextInput (Rectangle *r, Color c, char *text, int fontSize);