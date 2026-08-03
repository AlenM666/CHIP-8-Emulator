#include "window/window.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    const char *from_path = argc > 1 ? argv[1] : "game/ibm.ch8";

    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [rom-path]\n", argv[0]);
        return 2;
    }

    create_window();
    return 0;
}


