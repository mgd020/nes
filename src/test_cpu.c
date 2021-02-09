#define DEBUG
#include "cpu.h"
#include "bus.h"
#include "ram.h"
#include "test.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

#define LEN(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

#define CLEAR_SCREEN "\e[1;1H\e[2J"

int main()
{
    CPU cpu;
    Bus bus;
    RAM ram;
    RAM cart;

    Bus_init(&bus);
    CPU_init(&cpu, &bus);
    RAM_init(&ram, 0x800, 0, 0x1FFF);
    RAM_init(&cart, 0xBFE0, 0x4020, 0xFFFF);

    Bus_connect(&bus, (BusDevice*) &cpu);
    Bus_connect(&bus, (BusDevice*) &ram);
    Bus_connect(&bus, (BusDevice*) &cart);

    /*
        https://www.masswerk.at/6502/assembler.html

        *=$8000
        LDX #10
        STX $0000
        LDX #3
        STX $0001
        LDY $0000
        LDA #0
        CLC
        loop
        ADC $0001
        DEY
        BNE loop
        STA $0002
        NOP
        NOP
        NOP

        8000        LDX #$0A        A2 0A
        8002        STX $0000       8E 00 00
        8005        LDX #$03        A2 03
        8007        STX $0001       8E 01 00
        800A        LDY $0000       AC 00 00
        800D        LDA #$00        A9 00
        800F        CLC             18
        8010 LOOP   
        8010        ADC $0001       6D 01 00
        8013        DEY             88
        8014        BNE LOOP        D0 FA
        8016        STA $0002       8D 02 00
        8019        NOP             EA
        801A        NOP             EA
        801B        NOP             EA
    */
    static const unsigned char program[] = {
        0xA2,
        0x0A,
        0x8E,
        0x00,
        0x00,
        0xA2,
        0x03,
        0x8E,
        0x01,
        0x00,
        0xAC,
        0x00,
        0x00,
        0xA9,
        0x00,
        0x18,
        0x6D,
        0x01,
        0x00,
        0x88,
        0xD0,
        0xFA,
        0x8D,
        0x02,
        0x00,
        0xEA,
        0xEA,
        0xEA,
    };

    for (int i = 0, e = LEN(program); i < e; ++i)
    {
        RAM_write(&cart, i + 0x8000, program[i]);
    }

    RAM_write(&cart, 0xFFFD, 0x80);

    CPU_reset(&cpu);

#if 1
    // it should take ~40 cycles to compute the result
    for (int i = 0; i < 40; ++i)
    {
        cpu.cycles = 0; // skip wait
        Bus_tick(&bus);
    }

    int result = RAM_read(&ram, 2);
    assert(result == 0x1E);

#else
    // interactive
    for (;;)
    {
        puts(CLEAR_SCREEN);
        print_cpu(&cpu);
        puts("");
        print_memory(&bus, 0, 16);
        puts("");
        disassemble(&bus, cpu.pc, 16);
        getchar();
        cpu.cycles = 0; // skip wait
        Bus_tick(&bus);
    }
#endif

    return 0;
}
