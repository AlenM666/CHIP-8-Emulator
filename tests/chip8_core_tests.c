#include "../src/chip8/chip8.h"

#include <assert.h>
#include <stdio.h>

static void put_opcode(Chip8 *chip8, uint16_t address, uint16_t opcode)
{
    chip8->memory[address] = (uint8_t)(opcode >> 8u);
    chip8->memory[address + 1u] = (uint8_t)(opcode & 0xFFu);
}

static void test_init(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    assert(chip8.pc == CHIP8_PROGRAM_START);
    assert(chip8.memory[CHIP8_FONT_START] == 0xF0u);
    assert(chip8.memory[CHIP8_FONT_START + 79u] == 0x80u);
}

static void test_arithmetic_and_carry(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    put_opcode(&chip8, 0x200u, 0x61FEu); // V1 = 254
    put_opcode(&chip8, 0x202u, 0x6203u); // V2 = 3
    put_opcode(&chip8, 0x204u, 0x8124u); // V1 += V2, VF = carry

    assert(chip8_cycle(&chip8));
    assert(chip8_cycle(&chip8));
    assert(chip8_cycle(&chip8));

    assert(chip8.registers[1] == 1u);
    assert(chip8.registers[0xF] == 1u);
}

static void test_call_and_return(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    put_opcode(&chip8, 0x200u, 0x2300u); // call 0x300
    put_opcode(&chip8, 0x300u, 0x00EEu); // return

    assert(chip8_cycle(&chip8));
    assert(chip8.pc == 0x300u);
    assert(chip8.sp == 1u);

    assert(chip8_cycle(&chip8));
    assert(chip8.pc == 0x202u);
    assert(chip8.sp == 0u);
}

static void test_draw_collision(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.index = 0x300u;
    chip8.memory[0x300u] = 0x80u;
    chip8.registers[1] = 0u;
    chip8.registers[2] = 0u;

    put_opcode(&chip8, 0x200u, 0xD121u);
    put_opcode(&chip8, 0x202u, 0xD121u);

    assert(chip8_cycle(&chip8));
    assert(chip8.video[0] == 0xFFFFFFFFu);
    assert(chip8.registers[0xF] == 0u);

    assert(chip8_cycle(&chip8));
    assert(chip8.video[0] == 0u);
    assert(chip8.registers[0xF] == 1u);
}

static void test_bcd(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    chip8.registers[3] = 197u;
    chip8.index = 0x300u;
    put_opcode(&chip8, 0x200u, 0xF333u);

    assert(chip8_cycle(&chip8));
    assert(chip8.memory[0x300u] == 1u);
    assert(chip8.memory[0x301u] == 9u);
    assert(chip8.memory[0x302u] == 7u);
}

static void test_key_wait(void)
{
    Chip8 chip8;
    chip8_init(&chip8);

    put_opcode(&chip8, 0x200u, 0xF00Au);
    put_opcode(&chip8, 0x202u, 0x6101u);

    assert(chip8_cycle(&chip8));
    assert(chip8.waiting_for_key);
    assert(chip8.pc == 0x202u);

    assert(chip8_cycle(&chip8));
    assert(chip8.pc == 0x202u);

    chip8_set_key(&chip8, 0xAu, true);
    assert(!chip8.waiting_for_key);
    assert(chip8.registers[0] == 0xAu);

    assert(chip8_cycle(&chip8));
    assert(chip8.registers[1] == 1u);
}

int main(void)
{
    test_init();
    test_arithmetic_and_carry();
    test_call_and_return();
    test_draw_collision();
    test_bcd();
    test_key_wait();

    puts("chip8_core_tests: ok");
    return 0;
}

