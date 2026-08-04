#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const uint8_t FONTSET[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

static uint8_t random_byte(void)
{
    return (uint8_t)(rand() & 0xFF);
}

static void clear_display(Chip8 *chip8)
{
    memset(chip8->video, 0, sizeof(chip8->video));
    chip8->draw_flag = true;
}

void chip8_init(Chip8 *chip8)
{
    memset(chip8, 0, sizeof(*chip8));
    chip8->pc = CHIP8_PROGRAM_START;
    memcpy(&chip8->memory[CHIP8_FONT_START], FONTSET, sizeof(FONTSET));
    srand((unsigned int)time(NULL));
}

bool chip8_load_rom(Chip8 *chip8, const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Failed to open ROM: %s\n", filename);
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "Failed to seek ROM: %s\n", filename);
        fclose(file);
        return false;
    }

    const long size = ftell(file);
    if (size <= 0)
    {
        fprintf(stderr, "ROM is empty or unreadable: %s\n", filename);
        fclose(file);
        return false;
    }

    if ((size_t)size > CHIP8_MEMORY_SIZE - CHIP8_PROGRAM_START)
    {
        fprintf(stderr, "ROM too large: %ld bytes, max %u bytes\n", size,
                CHIP8_MEMORY_SIZE - CHIP8_PROGRAM_START);
        fclose(file);
        return false;
    }

    rewind(file);
    const size_t read_bytes = fread(&chip8->memory[CHIP8_PROGRAM_START], 1, (size_t)size, file);
    fclose(file);

    if (read_bytes != (size_t)size)
    {
        fprintf(stderr, "Failed to read complete ROM: %s\n", filename);
        return false;
    }

    return true;
}

void chip8_tick_timers(Chip8 *chip8)
{
    if (chip8->delay_timer > 0)
    {
        --chip8->delay_timer;
    }

    if (chip8->sound_timer > 0)
    {
        --chip8->sound_timer;
    }
}

void chip8_set_key(Chip8 *chip8, uint8_t key, bool pressed)
{
    if (key >= CHIP8_KEY_COUNT)
    {
        return;
    }

    chip8->keypad[key] = pressed ? 1u : 0u;

    if (pressed && chip8->waiting_for_key)
    {
        chip8->registers[chip8->waiting_register] = key;
        chip8->waiting_for_key = false;
    }
}

static bool push_stack(Chip8 *chip8, uint16_t value)
{
    if (chip8->sp >= CHIP8_STACK_SIZE)
    {
        fprintf(stderr, "CHIP-8 stack overflow at PC=0x%03X\n", chip8->pc);
        return false;
    }

    chip8->stack[chip8->sp++] = value;
    return true;
}

static bool pop_stack(Chip8 *chip8, uint16_t *value)
{
    if (chip8->sp == 0)
    {
        fprintf(stderr, "CHIP-8 stack underflow at PC=0x%03X\n", chip8->pc);
        return false;
    }

    *value = chip8->stack[--chip8->sp];
    return true;
}

bool chip8_cycle(Chip8 *chip8)
{
    if (chip8->waiting_for_key)
    {
        return true;
    }

    if ((size_t)chip8->pc + 1u >= CHIP8_MEMORY_SIZE)
    {
        fprintf(stderr, "Program counter out of bounds: 0x%03X\n", chip8->pc);
        return false;
    }

    chip8->opcode = (uint16_t)((chip8->memory[chip8->pc] << 8u) | chip8->memory[chip8->pc + 1u]);
    chip8->pc = (uint16_t)(chip8->pc + 2u);

    const uint16_t nnn = chip8->opcode & 0x0FFFu;
    const uint8_t nn = (uint8_t)(chip8->opcode & 0x00FFu);
    const uint8_t n = (uint8_t)(chip8->opcode & 0x000Fu);
    const uint8_t x = (uint8_t)((chip8->opcode & 0x0F00u) >> 8u);
    const uint8_t y = (uint8_t)((chip8->opcode & 0x00F0u) >> 4u);

    switch (chip8->opcode & 0xF000u)
    {
        case 0x0000:
            switch (chip8->opcode)
            {
                case 0x00E0:
                    clear_display(chip8);
                    break;
                case 0x00EE:
                    if (!pop_stack(chip8, &chip8->pc))
                    {
                        return false;
                    }
                    break;
                default:
                    // 0NNN is ignored by modern CHIP-8 interpreters.
                    break;
            }
            break;

        case 0x1000:
            chip8->pc = nnn;
            break;

        case 0x2000:
            if (!push_stack(chip8, chip8->pc))
            {
                return false;
            }
            chip8->pc = nnn;
            break;

        case 0x3000:
            if (chip8->registers[x] == nn)
            {
                chip8->pc = (uint16_t)(chip8->pc + 2u);
            }
            break;

        case 0x4000:
            if (chip8->registers[x] != nn)
            {
                chip8->pc = (uint16_t)(chip8->pc + 2u);
            }
            break;

        case 0x5000:
            if (n == 0u && chip8->registers[x] == chip8->registers[y])
            {
                chip8->pc = (uint16_t)(chip8->pc + 2u);
            }
            break;

        case 0x6000:
            chip8->registers[x] = nn;
            break;

        case 0x7000:
            chip8->registers[x] = (uint8_t)(chip8->registers[x] + nn);
            break;

        case 0x8000:
            switch (n)
            {
                case 0x0:
                    chip8->registers[x] = chip8->registers[y];
                    break;
                case 0x1:
                    chip8->registers[x] = (uint8_t)(chip8->registers[x] | chip8->registers[y]);
                    chip8->registers[0xF] = 0;
                    break;
                case 0x2:
                    chip8->registers[x] = (uint8_t)(chip8->registers[x] & chip8->registers[y]);
                    chip8->registers[0xF] = 0;
                    break;
                case 0x3:
                    chip8->registers[x] = (uint8_t)(chip8->registers[x] ^ chip8->registers[y]);
                    chip8->registers[0xF] = 0;
                    break;
                case 0x4:
                {
                    const uint16_t sum = (uint16_t)chip8->registers[x] + chip8->registers[y];
                    chip8->registers[x] = (uint8_t)sum;
                    chip8->registers[0xF] = (sum > 0xFFu) ? 1u : 0u;
                    break;
                }
                case 0x5:
                {
                    const uint8_t vx = chip8->registers[x];
                    chip8->registers[x] = (uint8_t)(vx - chip8->registers[y]);
                    chip8->registers[0xF] = (vx >= chip8->registers[y]) ? 1u : 0u;
                    break;
                }
                case 0x6:
                    // Original COSMAC VIP behavior: copy VY to VX, then shift VX.
                    chip8->registers[x] = chip8->registers[y];
                    chip8->registers[0xF] = (uint8_t)(chip8->registers[x] & 0x1u);
                    chip8->registers[x] = (uint8_t)(chip8->registers[x] >> 1u);
                    break;
                case 0x7:
                {
                    const uint8_t vx = chip8->registers[x];
                    chip8->registers[x] = (uint8_t)(chip8->registers[y] - vx);
                    chip8->registers[0xF] = (chip8->registers[y] >= vx) ? 1u : 0u;
                    break;
                }
                case 0xE:
                    // Original COSMAC VIP behavior: copy VY to VX, then shift VX.
                    chip8->registers[x] = chip8->registers[y];
                    chip8->registers[0xF] = (uint8_t)((chip8->registers[x] & 0x80u) >> 7u);
                    chip8->registers[x] = (uint8_t)(chip8->registers[x] << 1u);
                    break;
                default:
                    fprintf(stderr, "Unknown opcode: 0x%04X\n", chip8->opcode);
                    return false;
            }
            break;

        case 0x9000:
            if (n == 0u && chip8->registers[x] != chip8->registers[y])
            {
                chip8->pc = (uint16_t)(chip8->pc + 2u);
            }
            break;

        case 0xA000:
            chip8->index = nnn;
            break;

        case 0xB000:
            // Original behavior: jump to NNN + V0.
            chip8->pc = (uint16_t)(nnn + chip8->registers[0]);
            break;

        case 0xC000:
            chip8->registers[x] = (uint8_t)(random_byte() & nn);
            break;

        case 0xD000:
        {
            const uint8_t vx = (uint8_t)(chip8->registers[x] % CHIP8_DISPLAY_WIDTH);
            const uint8_t vy = (uint8_t)(chip8->registers[y] % CHIP8_DISPLAY_HEIGHT);
            chip8->registers[0xF] = 0;

            for (uint8_t row = 0; row < n; ++row)
            {
                if ((uint16_t)(chip8->index + row) >= CHIP8_MEMORY_SIZE)
                {
                    break;
                }

                const uint8_t sprite_byte = chip8->memory[chip8->index + row];
                const uint8_t py = (uint8_t)(vy + row);
                if (py >= CHIP8_DISPLAY_HEIGHT)
                {
                    continue;
                }

                for (uint8_t col = 0; col < 8u; ++col)
                {
                    const uint8_t px = (uint8_t)(vx + col);
                    if (px >= CHIP8_DISPLAY_WIDTH)
                    {
                        continue;
                    }

                    const uint8_t sprite_pixel = (uint8_t)(sprite_byte & (0x80u >> col));
                    if (sprite_pixel == 0u)
                    {
                        continue;
                    }

                    const size_t pixel_index = (size_t)py * CHIP8_DISPLAY_WIDTH + px;
                    if (chip8->video[pixel_index] != 0u)
                    {
                        chip8->registers[0xF] = 1;
                    }
                    chip8->video[pixel_index] ^= 0xFFFFFFFFu;
                }
            }

            chip8->draw_flag = true;
            break;
        }

        case 0xE000:
            switch (nn)
            {
                case 0x9E:
                    if (chip8->registers[x] < CHIP8_KEY_COUNT && chip8->keypad[chip8->registers[x]] != 0u)
                    {
                        chip8->pc = (uint16_t)(chip8->pc + 2u);
                    }
                    break;
                case 0xA1:
                    if (chip8->registers[x] >= CHIP8_KEY_COUNT || chip8->keypad[chip8->registers[x]] == 0u)
                    {
                        chip8->pc = (uint16_t)(chip8->pc + 2u);
                    }
                    break;
                default:
                    fprintf(stderr, "Unknown opcode: 0x%04X\n", chip8->opcode);
                    return false;
            }
            break;

        case 0xF000:
            switch (nn)
            {
                case 0x07:
                    chip8->registers[x] = chip8->delay_timer;
                    break;
                case 0x0A:
                    chip8->waiting_for_key = true;
                    chip8->waiting_register = x;
                    break;
                case 0x15:
                    chip8->delay_timer = chip8->registers[x];
                    break;
                case 0x18:
                    chip8->sound_timer = chip8->registers[x];
                    break;
                case 0x1E:
                    chip8->index = (uint16_t)(chip8->index + chip8->registers[x]);
                    chip8->registers[0xF] = chip8->index > 0x0FFFu ? 1u : 0u;
                    chip8->index &= 0x0FFFu;
                    break;
                case 0x29:
                    chip8->index = (uint16_t)(CHIP8_FONT_START + (chip8->registers[x] & 0x0Fu) * 5u);
                    break;
                case 0x33:
                    if ((size_t)chip8->index + 2u >= CHIP8_MEMORY_SIZE)
                    {
                        fprintf(stderr, "BCD store out of bounds at I=0x%03X\n", chip8->index);
                        return false;
                    }
                    chip8->memory[chip8->index] = (uint8_t)(chip8->registers[x] / 100u);
                    chip8->memory[chip8->index + 1u] = (uint8_t)((chip8->registers[x] / 10u) % 10u);
                    chip8->memory[chip8->index + 2u] = (uint8_t)(chip8->registers[x] % 10u);
                    break;
                case 0x55:
                    if ((size_t)chip8->index + x >= CHIP8_MEMORY_SIZE)
                    {
                        fprintf(stderr, "Register dump out of bounds at I=0x%03X\n", chip8->index);
                        return false;
                    }
                    for (uint8_t r = 0; r <= x; ++r)
                    {
                        chip8->memory[chip8->index + r] = chip8->registers[r];
                    }
                    // Original COSMAC VIP behavior increments I after FX55/FX65.
                    chip8->index = (uint16_t)(chip8->index + x + 1u);
                    break;
                case 0x65:
                    if ((size_t)chip8->index + x >= CHIP8_MEMORY_SIZE)
                    {
                        fprintf(stderr, "Register load out of bounds at I=0x%03X\n", chip8->index);
                        return false;
                    }
                    for (uint8_t r = 0; r <= x; ++r)
                    {
                        chip8->registers[r] = chip8->memory[chip8->index + r];
                    }
                    // Original COSMAC VIP behavior increments I after FX55/FX65.
                    chip8->index = (uint16_t)(chip8->index + x + 1u);
                    break;
                default:
                    fprintf(stderr, "Unknown opcode: 0x%04X\n", chip8->opcode);
                    return false;
            }
            break;

        default:
            fprintf(stderr, "Unknown opcode: 0x%04X\n", chip8->opcode);
            return false;
    }

    return true;
}

