#include <stdio.h>

#include "6502.h"
#include "cpu.c"

void disassemble(Bus *bus, int addr, int lines)
{
    /*
Example:

8000        LDX #$0A        A2 0A
8002        STX $0000       8E 00 00
8005        LDX #$03        A2 03
8007        STX $0001       8E 01 00
800A        LDY $0000       AC 00 00
800D        LDA #$00        A9 00
800F        CLC             18
8010        ADC $0001       6D 01 00
8013        DEY             88
8014        BNE $8010       D0 FA
8016        STA $0002       8D 02 00
8019        NOP             EA
801A        NOP             EA
801B        NOP             EA
    */
    while (lines-- > 0)
    {
        int lo, hi, op = Bus_read(bus, addr);
        switch (op)
        {

#define X(INSTRUCTION, ADDRESSING_MODE, OPCODE, CYCLES)                                                            \
    case OPCODE:                                                                                                   \
        if (ADDRESSING_MODE == IMP)                                                                                \
        {                                                                                                          \
            printf("%04X        " #INSTRUCTION "             %02X\n", addr, op);                                   \
            ++addr;                                                                                                \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ACC)                                                                           \
        {                                                                                                          \
            printf("%04X        " #INSTRUCTION " A           %02X\n", addr, op);                                   \
            ++addr;                                                                                                \
        }                                                                                                          \
        else if (ADDRESSING_MODE == IMM)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            printf("%04X        " #INSTRUCTION " #$%02X        %02X %02X\n", addr, lo, op, lo);                    \
            addr += 2;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ZPG)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X         %02X %02X\n", addr, lo, op, lo);                    \
            addr += 2;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ZPX)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X,X       %02X %02X\n", addr, lo, op, lo);                    \
            addr += 2;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ZPY)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X,Y       %02X %02X\n", addr, lo, op, lo);                    \
            addr += 2;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == REL)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            printf("%04X        " #INSTRUCTION " $%04X       %02X %02X\n", addr, addr + u8_to_s8(lo) + 2, op, lo); \
            addr += 2;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ABS)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            hi = Bus_read(bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X%02X       %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ABX)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            hi = Bus_read(bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X%02X,X     %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ABY)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            hi = Bus_read(bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X%02X,Y     %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == IND)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            hi = Bus_read(bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " ($%02X%02X)     %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == INX)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            hi = Bus_read(bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " ($%02X%02X,X)   %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == INY)                                                                           \
        {                                                                                                          \
            lo = Bus_read(bus, addr + 1);                                                                     \
            hi = Bus_read(bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " ($%02X%02X),Y   %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        break;

            INSTRUCTION_SET();

#undef X
        }
    }
}

void print_memory(Bus *bus, int addr, int lines)
{
    while (lines-- > 0)
    {
        printf("%04X: ", addr);
        for (int i = 0; i < 16; ++i)
        {
            printf("%02X ", Bus_read(bus, addr + i));
        }
        printf("\n");
        addr += 16;
    }
}

void print_cpu(CPU *cpu)
{
    printf("         n v - b d i z c\n");
    printf("status:  %d %d   %d %d %d %d %d\n", cpu->n, cpu->v, cpu->b, cpu->d, cpu->i, cpu->z, cpu->c);
    printf("pc:      $%04X\n", cpu->pc);
    printf("sp:      $1%02X\n", cpu->sp);
    printf("cycles:  %d\n", cpu->cycles);
    printf("          a      x      y\n");
    printf("regs:    #%02X    #%02X    #%02X\n", cpu->a, cpu->x, cpu->y);
}
