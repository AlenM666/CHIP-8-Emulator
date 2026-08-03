#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#define CHIP8_DISPLAY_WIDTH 64u
#define CHIP8_DISPLAY_HEIGHT 32u
#define CHIP8_DISPLAY_PIXELS (CHIP8_DISPLAY_WIDTH * CHIP8_DISPLAY_HEIGHT)

#define CHIP8_MEMORY_SIZE 4096u
#define CHIP8_REGISTER_COUNT 16u
#define CHIP8_KEY_COUNT 16u
#define CHIP8_STACK_SIZE 16u
#define CHIP8_PROGRAM_START 0x200u
#define CHIP8_FONT_START 0x050u


typedef struct 
{
    uint8_t memory[CHIP8_MEMORY_SIZE];
    uint8_t registers[CHIP8_REGISTER_COUNT];
    uint8_t sp;
    uint8_t delay_timer;
    uint8_t soudn_timer;
    uint8_t keypad[CHIP8_KEY_COUNT];
    uint16_t index;
    uint16_t pc;
    uint16_t stack[CHIP8_STACK_SIZE];
    uint32_t video[CHIP8_DISPLAY_PIXELS];
    bool draw_flag;
    bool waiting_for_key;
}Chip8;

void chip8_init(Chip8 *chip8);
void chip8_load_rom(Chip8 *chip8, const char *filename);

static uint8_t random_byte(void);
static void clear_display(Chip8 *chip8);


#endif // !CHIP8_H
