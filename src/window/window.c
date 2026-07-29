/* #include "window.h" */
#include "raylib.h"
#include "window.h"

#define WIDTH 800
#define HEIGHT 600
#define TITLE "Musi"
#define FPS 60

void draw_loop(void)
{
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }
    
}

void create_window(void)
{
    InitWindow(WIDTH, HEIGHT, TITLE);
    SetTargetFPS(FPS);

    draw_loop();
    close_window();
}

void close_window(void)
{
    CloseWindow();
}
