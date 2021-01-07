#pragma once

typedef struct Bus
{
    unsigned char ram[0x10000];
} Bus;

void Bus_init(Bus *bus);

// Read a byte from the bus.
int Bus_read(Bus *bus, int addr);

// Write a byte to the bus.
int Bus_write(Bus *bus, int addr, int byte);
