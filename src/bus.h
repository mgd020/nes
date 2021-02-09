#pragma once

typedef struct BusDevice BusDevice;

typedef void (*BusDeviceTick)(BusDevice *device);
typedef int (*BusDeviceRead)(BusDevice *device, int addr);
typedef int (*BusDeviceWrite)(BusDevice *device, int addr, int byte);

/*
    An interface to a device on the bus.

    For tick(), each device's tick() method is called.

    For read() and write(), the first device that returns a value != ~0 is
    accepted and returned to the caller of the bus method.
*/
struct BusDevice
{
    struct BusDevice *next;
    BusDeviceTick tick;
    BusDeviceRead read;
    BusDeviceWrite write;
};

typedef struct Bus
{
    BusDevice *devices;
} Bus;

void Bus_init(Bus *bus);

// Connect a device to the bus
int Bus_connect(Bus *bus, BusDevice *device);

// Remove a device from the bus.
int Bus_disconnect(Bus *bus, BusDevice *device);

// Send tick signal to all devices.
void Bus_tick(Bus *bus);

// Read a byte from the bus.
int Bus_read(Bus *bus, int addr);

// Write a byte to the bus.
int Bus_write(Bus *bus, int addr, int byte);
