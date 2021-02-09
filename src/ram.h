#pragma once

#include "bus.h"

typedef struct RAM
{
    BusDevice device;

    unsigned char *bytes;
    int size, addr_min, addr_max;
} RAM;

void RAM_init(RAM *ram, int size, int addr_min, int addr_max);

// Read byte at address, returning -1 on failure.
int RAM_read(RAM *ram, int addr);

// Write byte at address, returning -1 on failure.
int RAM_write(RAM *ram, int addr, int byte);
