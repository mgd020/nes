#pragma once

typedef struct CPU CPU;
typedef struct Bus Bus;

void print_cpu(CPU *cpu);
void print_memory(Bus *bus, int addr, int lines);
void disassemble(Bus *bus, int addr, int lines);
