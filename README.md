# CHIP-8 Emulator (C + raylib)

A classic CHIP-8 interpreter/emulator written in C99, rendered with [raylib](https://www.raylib.com/).

## Requirements

- `clang`
- `raylib` installed with fetch content Cmake
- `cmake`

### Installing raylib
if you want to install it system wide.

- **macOS (Homebrew):** `brew install raylib`
- **Ubuntu/Debian:** `sudo apt install libraylib-dev` (or build from source per raylib's docs)
- **Arch:** `sudo pacman -S raylib`



## Getting a ROM

Download a CHIP-8 ROM, e.g. `pong.rom`, and place it in the `game/` folder:

```
chip8/game/pong.rom
```

A good public domain source is the [CHIP-8 Archive](https://github.com/kripod/chip8-roms) or the [Chip-8 test suite](https://github.com/Timendus/chip8-test-suite).



## Build & Run
Run the run.sh file

```bash
$ ./run.sh
$ sh run.sh
```

If no ROM path is passed on the command line, the emulator defaults to `game/pong.rom`.




## Memory Map


```
+---------------+= 0xFFF (4095) End of Chip-8 RAM
|               |
|               |
|               |
|               |
|               |
| 0x200 to 0xFFF|
|     Chip-8    |
| Program / Data|
|     Space     |
|               |
|               |
|               |
+- - - - - - - -+= 0x600 (1536) Start of ETI 660 Chip-8 programs
|               |
|               |
|               |
+---------------+= 0x200 (512) Start of most Chip-8 programs
| 0x000 to 0x1FF|
| Reserved for  |
|  interpreter  |
+---------------+= 0x000 (0) Start of Chip-8 RAM

```



<br>


## Controls

The original CHIP-8 keypad is mapped onto your keyboard like this:

```
CHIP-8 Keypad          Keyboard
1 2 3 C                1 2 3 4
4 5 6 D      -->        Q W E R
7 8 9 E                 A S D F
A 0 B F                 Z X C V
```

For Pong specifically, the common controls are:
- Player 1 (left paddle): `1` (up) / `Q` (down)
- Player 2 (right paddle): `4` (up) / `R` (down)

(Exact keys depend on the ROM variant — check the ROM's documentation if paddles don't move as expected.)

## Notes

- The CPU runs at roughly 700 instructions/second, with timers ticking at 60Hz, which plays well with most classic ROMs.
- Sound: if you add a `resources/beep.wav` file, it will play whenever the CHIP-8 sound timer is active. This is optional — the emulator runs fine without it (silently).


## Resources
- https://en.wikipedia.org/wiki/CHIP-8
- https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
- https://jborza.com/post/2020-12-07-chip-8/
