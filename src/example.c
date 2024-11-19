#include <stdio.h>

#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "lt-filetree.h"

int main(int argc, char *argv[]) {
    InitWindow(1280, 720, "lt-raylib-file-tree");

    SetTargetFPS(60);

    Vector2 scroll = {0};
    Rectangle view = {0};

    FileTree file_tree = LTLoadFileTree(GetWorkingDirectory());
    const char* clicked_file_path = NULL;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (LTGuiFileTree(&file_tree, (Rectangle) { 0, 0, 500, 720 }, &scroll, &view, &clicked_file_path)) {
            printf("%s\n", clicked_file_path);
        }

        EndDrawing();
    }

    LTFreeFileTree(&file_tree);

    CloseWindow();
    return 0;
}