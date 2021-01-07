#include "bus.h"
#include <string.h>

void Bus_init(Bus *bus)
{
    memset(bus->ram, 0, 0x10000);
}

int Bus_read(Bus *bus, int addr)
{
    return bus->ram[addr & 0xFFFF];
}

int Bus_write(Bus *bus, int addr, int byte)
{
    bus->ram[addr & 0xFFFF] = byte & 0xFF;
    return 0;
}
