#pragma once

typedef struct Bus Bus;

typedef struct CPU
{
    Bus *bus;

    unsigned pc : 16; // program counter
    unsigned sp : 8;  // stack pointer (0x1xx)
    unsigned a : 8;   // accumulator
    unsigned x : 8;   // index register x
    unsigned y : 8;   // index register y
    unsigned n : 1;   // negative flag
    unsigned v : 1;   // overflow flag
    unsigned b : 1;   // break flag
    unsigned d : 1;   // decimal flag
    unsigned i : 1;   // interrupt disable flag
    unsigned z : 1;   // zero flag
    unsigned c : 1;   // carry flag

    // current instruction
    unsigned address : 16; // address
    unsigned cycles : 4;   // number of cycles left
    unsigned mode : 2;     // type of addressing mode
} CPU;

void CPU_init(CPU *, Bus *);

void CPU_step(CPU *); // maybe this should be called by the Bus? as a tick?

// Trigger reset
void CPU_reset(CPU *);

// Trigger interrupt request
void CPU_irq(CPU *);

// Trigger non-maskable interrupt
void CPU_nmi(CPU *);