#pragma once

typedef void (*BusDeviceTick)(void *ptr);
typedef int (*BusDeviceRead)(void *ptr, int addr);
typedef int (*BusDeviceWrite)(void *ptr, int addr, int byte);

/*
    An interface to a device on the bus.

    For tick(), each device's tick() method is called.

    For read() and write(), the first device that returns a value != ~0 is
    accepted and returned to the caller of the bus method.
*/
typedef struct BusDevice
{
    struct BusDevice *next;
    BusDeviceTick tick;
    BusDeviceRead read;
    BusDeviceWrite write;
    void *ptr;
} BusDevice;

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
