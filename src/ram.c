#include "ram.h"

#include <stdlib.h>

// support mirroring
#define REAL_ADDR(ADDR) (((ADDR) - ram->addr_min) % ram->size)

void RAM_message(RAM *ram, Bus *bus)
{
    int addr;

    switch (bus->message)
    {
    case BUS_READ:
        addr = bus->addr;
        if (addr >= ram->addr_min && addr <= ram->addr_max)
        {
            bus->data = ram->bytes[REAL_ADDR(addr)];
        }
        break;
    
    case BUS_WRITE:
        addr = bus->addr;
        if (addr >= ram->addr_min && addr <= ram->addr_max)
        {
            ram->bytes[REAL_ADDR(addr)] = bus->data;
        }
        break;
    
    default:
        break;
    }
}

void RAM_init(RAM *ram, int size, int addr_min, int addr_max)
{
    ram->bytes = malloc(size);
    ram->size = size;
    ram->addr_min = addr_min;
    ram->addr_max = addr_max;

    ram->device.message = (BusDeviceMessage) &RAM_message;
}
