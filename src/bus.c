#include "bus.h"
#include <assert.h>

void Bus_init(Bus *bus)
{
    bus->devices = 0;
}

int Bus_connect(Bus *bus, BusDevice *device)
{
    assert(device);
    device->next = bus->devices;
    bus->devices = device;
    return 0;
}

int Bus_disconnect(Bus *bus, BusDevice *device)
{
    assert(device);
    BusDevice *previous_device = bus->devices;
    if (previous_device == device)
    {
        bus->devices = device->next;
    }
    else
    {
        while (previous_device && previous_device->next != device)
        {
            previous_device = previous_device->next;
        }
        if (!previous_device)
        {
            // device not found
            return -1;
        }
        previous_device->next = device->next;
    }
    device->next = 0;
    return 0;
}

void Bus_tick(Bus *bus)
{
    for (BusDevice *device = bus->devices; device; device = device->next)
    {
        if (device->tick)
        {
            device->tick(device);
        }
    }
}

int Bus_read(Bus *bus, int addr)
{
    for (BusDevice *device = bus->devices; device; device = device->next)
    {
        if (!device->read)
        {
            continue;
        }

        int result = device->read(device, addr);

        if (result != ~0)
        {
            return result;
        }
    }

    return ~0;
}

int Bus_write(Bus *bus, int addr, int byte)
{
    for (BusDevice *device = bus->devices; device; device = device->next)
    {
        if (!device->write)
        {
            continue;
        }

        int result = device->write(device, addr, byte);

        if (result != ~0)
        {
            return result;
        }
    }

    return ~0;
}
