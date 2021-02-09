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

void Bus_message(Bus *bus, Message message)
{
    bus->message = message;

    for (BusDevice *device = bus->devices; device; device = device->next)
    {
        device->message(device, bus);
    }
}
