#pragma once

typedef struct Bus Bus;
typedef struct BusDevice BusDevice;

typedef void (*BusDeviceMessage)(BusDevice *device, Bus *bus);

struct BusDevice
{
    struct BusDevice *next;
    BusDeviceMessage message;
};

typedef enum Message
{
    // Tick
    BUS_TICK,

    // Read a byte
    BUS_READ,

    // Write a byte
    BUS_WRITE,

    // Non-maskable interrupt (vblank)
    BUS_NMI,

    // Interrupt
    BUS_IRQ,

    // Reset
    BUS_RESET,

} Message;

typedef struct Bus
{
    BusDevice *devices;
    Message message;
    int addr;
    int data;
} Bus;

void Bus_init(Bus *bus);

// Connect a device to the bus
int Bus_connect(Bus *bus, BusDevice *device);

// Remove a device from the bus.
int Bus_disconnect(Bus *bus, BusDevice *device);

// Send a message to every device on the bus
void Bus_message(Bus *bus, Message message);

// Read a byte from the bus (convenience method)
static inline int Bus_read(Bus *bus, int addr)
{
    bus->addr = addr;
    Bus_message(bus, BUS_READ);
    return bus->data;
}

// Write a byte to the bus (convenience method)
static inline void Bus_write(Bus *bus, int addr, int byte)
{
    bus->addr = addr;
    bus->data = byte;
    Bus_message(bus, BUS_WRITE);
}
