#include "ram.h"

#include <stdlib.h>

// support mirroring
#define REAL_ADDR(ADDR) (((ADDR)-ram->addr_min) % ram->size)

void RAM_init(RAM *ram, int size, int addr_min, int addr_max)
{
    ram->bytes = malloc(size);
    ram->size = size;
    ram->addr_min = addr_min;
    ram->addr_max = addr_max;

    ram->device.tick = 0;
    ram->device.read = (BusDeviceRead) &RAM_read;
    ram->device.write = (BusDeviceWrite) &RAM_write;
}

int RAM_read(RAM *ram, int addr)
{
    if (addr >= ram->addr_min && addr <= ram->addr_max)
    {
        return ram->bytes[REAL_ADDR(addr)];
    }
    return ~0;
}

int RAM_write(RAM *ram, int addr, int byte)
{
    if (addr >= ram->addr_min && addr <= ram->addr_max)
    {
        ram->bytes[REAL_ADDR(addr)] = byte & 0xff;
        return 0;
    }
    return ~0;
}
