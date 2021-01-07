#define DEBUG
#include "cpu.c"
#include "bus.h"

#include <string.h>

#define LEN(ARRAY) (sizeof(ARRAY) / sizeof((ARRAY)[0]))

#define CLEAR_SCREEN "\e[1;1H\e[2J"

int main()
{
    CPU cpu;
    Bus bus;
    cpu.bus = &bus;

    Bus_init(&bus);
    CPU_init(&cpu, &bus);

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

    memcpy(bus.ram + 0x8000, &program, LEN(program));
    bus.ram[0xFFFD] = 0x80;

    CPU_reset(&cpu);

    for (;;)
    {
        puts(CLEAR_SCREEN);
        print_state(&cpu);
        getchar();
        cpu.cycles = 0; // fuck waiting
        CPU_step(&cpu);
    }

    return 0;
}
