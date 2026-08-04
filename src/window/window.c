
#include "window.h"

#include "../chip8/chip8.h"
#include "raylib.h"

#include <stdio.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define WINDOW_TITLE "CHIP-8"
#define TARGET_FPS 60
#define CYCLES_PER_SECOND 700
#define CYCLES_PER_FRAME (CYCLES_PER_SECOND / TARGET_FPS)

static const int KEYMAP[CHIP8_KEY_COUNT] = {
    KEY_X,     // 0
    KEY_ONE,   // 1
    KEY_TWO,   // 2
    KEY_THREE, // 3
    KEY_Q,     // 4
    KEY_W,     // 5
    KEY_E,     // 6
    KEY_A,     // 7
    KEY_S,     // 8
    KEY_D,     // 9
    KEY_Z,     // A
    KEY_C,     // B
    KEY_FOUR,  // C
    KEY_R,     // D
    KEY_F,     // E
    KEY_V      // F
};

static void update_keypad(Chip8 *chip8)
{
    for (uint8_t key = 0; key < CHIP8_KEY_COUNT; ++key)
    {
        chip8_set_key(chip8, key, IsKeyDown(KEYMAP[key]));
    }
}

static void draw_display(const Chip8 *chip8)
{
    const int scale = 12;
    const int display_width = (int)CHIP8_DISPLAY_WIDTH * scale;
    const int display_height = (int)CHIP8_DISPLAY_HEIGHT * scale;
    const int origin_x = (WINDOW_WIDTH - display_width) / 2;
    const int origin_y = (WINDOW_HEIGHT - display_height) / 2;

    DrawRectangle(origin_x - 4, origin_y - 4, display_width + 8, display_height + 8, DARKGRAY);

    for (int y = 0; y < (int)CHIP8_DISPLAY_HEIGHT; ++y)
    {
        for (int x = 0; x < (int)CHIP8_DISPLAY_WIDTH; ++x)
        {
            const size_t pixel_index = (size_t)y * CHIP8_DISPLAY_WIDTH + (size_t)x;
            const Color color = chip8->video[pixel_index] != 0u ? RAYWHITE : BLACK;
            DrawRectangle(origin_x + x * scale, origin_y + y * scale, scale, scale, color);
        }
    }
}

static void draw_overlay(const Chip8 *chip8, const char *rom_path, bool paused)
{
    DrawText("CHIP-8", 16, 14, 24, RAYWHITE);
    DrawText(TextFormat("ROM: %s", rom_path), 16, 44, 16, GRAY);
    DrawText("Keys: 1234/QWER/ASDF/ZXCV  |  P pause  |  R reset  |  ESC quit", 16, WINDOW_HEIGHT - 28, 16, GRAY);

    if (chip8->sound_timer > 0)
    {
        DrawText("BEEP", WINDOW_WIDTH - 78, 18, 20, YELLOW);
    }

    if (chip8->waiting_for_key)
    {
        DrawText("WAITING FOR KEY", WINDOW_WIDTH - 220, 48, 18, SKYBLUE);
    }

    if (paused)
    {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.45f));
        DrawText("PAUSED", WINDOW_WIDTH / 2 - 58, WINDOW_HEIGHT / 2 - 18, 36, YELLOW);
    }
}

int run_emulator(const char *rom_path)
{
    Chip8 chip8;
    chip8_init(&chip8);

    if (!chip8_load_rom(&chip8, rom_path))
    {
        return 1;
    }

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);

    bool paused = false;
    bool running = true;

    while (!WindowShouldClose() && running)
    {
        if (IsKeyPressed(KEY_P))
        {
            paused = !paused;
        }

        if (IsKeyPressed(KEY_R))
        {
            chip8_init(&chip8);
            running = chip8_load_rom(&chip8, rom_path);
        }

        update_keypad(&chip8);

        if (!paused)
        {
            for (int cycle = 0; cycle < CYCLES_PER_FRAME; ++cycle)
            {
                if (!chip8_cycle(&chip8))
                {
                    running = false;
                    break;
                }
            }
            chip8_tick_timers(&chip8);
        }

        BeginDrawing();
        ClearBackground(BLACK);
        draw_display(&chip8);
        draw_overlay(&chip8, rom_path, paused);
        EndDrawing();
    }

    CloseWindow();

    if (!running)
    {
        fprintf(stderr, "Emulation stopped after an unrecoverable CHIP-8 error.\n");
        return 1;
    }

    return 0;
}
