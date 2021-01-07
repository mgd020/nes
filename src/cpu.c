#include <assert.h>

#include "cpu.h"
#include "bus.h"

#ifndef NDEBUG
#include <stdio.h>
#endif

#define SP_BASE 0x100

// TODO: decimal?

// X(INSTRUCTION, ADDRESSING_MODE, OPCODE, CYCLES)
#define INSTRUCTION_SET() \
    X(ADC, IMM, 0x69, 2)  \
    X(ADC, ZPG, 0x65, 3)  \
    X(ADC, ZPX, 0x75, 4)  \
    X(ADC, ABS, 0x6d, 4)  \
    X(ADC, ABX, 0x7d, 4)  \
    X(ADC, ABY, 0x79, 4)  \
    X(ADC, IDX, 0x61, 6)  \
    X(ADC, IDY, 0x71, 5)  \
    X(AND, IMM, 0x29, 2)  \
    X(AND, ZPG, 0x25, 3)  \
    X(AND, ZPX, 0x35, 4)  \
    X(AND, ABS, 0x2d, 4)  \
    X(AND, ABX, 0x3d, 4)  \
    X(AND, ABY, 0x39, 4)  \
    X(AND, IDX, 0x21, 6)  \
    X(AND, IDY, 0x31, 5)  \
    X(ASL, ACC, 0x0a, 2)  \
    X(ASL, ZPG, 0x06, 5)  \
    X(ASL, ZPX, 0x16, 6)  \
    X(ASL, ABS, 0x0e, 6)  \
    X(ASL, ABX, 0x1e, 7)  \
    X(BCC, REL, 0x90, 2)  \
    X(BCS, REL, 0xb0, 2)  \
    X(BEQ, REL, 0xf0, 2)  \
    X(BIT, ZPG, 0x24, 3)  \
    X(BIT, ABS, 0x2c, 4)  \
    X(BMI, REL, 0x30, 2)  \
    X(BNE, REL, 0xd0, 2)  \
    X(BPL, REL, 0x10, 2)  \
    X(BRK, IMP, 0x00, 7)  \
    X(BVC, REL, 0x50, 2)  \
    X(BVS, REL, 0x70, 2)  \
    X(CLC, IMP, 0x18, 2)  \
    X(CLD, IMP, 0xd8, 2)  \
    X(CLI, IMP, 0x58, 2)  \
    X(CLV, IMP, 0xb8, 2)  \
    X(CMP, IMM, 0xc9, 2)  \
    X(CMP, ZPG, 0xc5, 3)  \
    X(CMP, ZPX, 0xd5, 4)  \
    X(CMP, ABS, 0xcd, 4)  \
    X(CMP, ABX, 0xdd, 4)  \
    X(CMP, ABY, 0xd9, 4)  \
    X(CMP, IDX, 0xc1, 6)  \
    X(CMP, IDY, 0xd1, 5)  \
    X(CPX, IMM, 0xe0, 2)  \
    X(CPX, ZPG, 0xe4, 3)  \
    X(CPX, ABS, 0xec, 4)  \
    X(CPY, IMM, 0xc0, 2)  \
    X(CPY, ZPG, 0xc4, 3)  \
    X(CPY, ABS, 0xcc, 4)  \
    X(DEC, ZPG, 0xc6, 5)  \
    X(DEC, ZPX, 0xd6, 6)  \
    X(DEC, ABS, 0xce, 6)  \
    X(DEC, ABX, 0xde, 7)  \
    X(DEX, IMP, 0xca, 2)  \
    X(DEY, IMP, 0x88, 2)  \
    X(EOR, IMM, 0x49, 2)  \
    X(EOR, ZPG, 0x45, 3)  \
    X(EOR, ZPX, 0x55, 4)  \
    X(EOR, ABS, 0x4d, 4)  \
    X(EOR, ABX, 0x5d, 4)  \
    X(EOR, ABY, 0x59, 4)  \
    X(EOR, IDX, 0x41, 6)  \
    X(EOR, IDY, 0x51, 5)  \
    X(INC, ZPG, 0xe6, 5)  \
    X(INC, ZPX, 0xf6, 6)  \
    X(INC, ABS, 0xee, 6)  \
    X(INC, ABX, 0xfe, 7)  \
    X(INX, IMP, 0xe8, 2)  \
    X(INY, IMP, 0xc8, 2)  \
    X(JMP, ABS, 0x4c, 3)  \
    X(JMP, IND, 0x6c, 5)  \
    X(JSR, ABS, 0x20, 6)  \
    X(LDA, IMM, 0xa9, 2)  \
    X(LDA, ZPG, 0xa5, 3)  \
    X(LDA, ZPX, 0xb5, 4)  \
    X(LDA, ABS, 0xad, 4)  \
    X(LDA, ABX, 0xbd, 4)  \
    X(LDA, ABY, 0xb9, 4)  \
    X(LDA, IDX, 0xa1, 6)  \
    X(LDA, IDY, 0xb1, 5)  \
    X(LDX, IMM, 0xa2, 2)  \
    X(LDX, ZPG, 0xa6, 3)  \
    X(LDX, ZPY, 0xb6, 4)  \
    X(LDX, ABS, 0xae, 4)  \
    X(LDX, ABY, 0xbe, 4)  \
    X(LDY, IMM, 0xa0, 2)  \
    X(LDY, ZPG, 0xa4, 3)  \
    X(LDY, ZPX, 0xb4, 4)  \
    X(LDY, ABS, 0xac, 4)  \
    X(LDY, ABX, 0xbc, 4)  \
    X(LSR, ACC, 0x4a, 2)  \
    X(LSR, ZPG, 0x46, 5)  \
    X(LSR, ZPX, 0x56, 6)  \
    X(LSR, ABS, 0x4e, 6)  \
    X(LSR, ABX, 0x5e, 7)  \
    X(NOP, IMP, 0xea, 2)  \
    X(ORA, IMM, 0x09, 2)  \
    X(ORA, ZPG, 0x05, 3)  \
    X(ORA, ZPX, 0x15, 4)  \
    X(ORA, ABS, 0x0d, 4)  \
    X(ORA, ABX, 0x1d, 4)  \
    X(ORA, ABY, 0x19, 4)  \
    X(ORA, IDX, 0x01, 6)  \
    X(ORA, IDY, 0x11, 5)  \
    X(PHA, IMP, 0x48, 3)  \
    X(PHP, IMP, 0x08, 3)  \
    X(PLA, IMP, 0x68, 4)  \
    X(PLP, IMP, 0x28, 4)  \
    X(ROL, ACC, 0x2a, 2)  \
    X(ROL, ZPG, 0x26, 5)  \
    X(ROL, ZPX, 0x36, 6)  \
    X(ROL, ABS, 0x2e, 6)  \
    X(ROL, ABX, 0x3e, 7)  \
    X(ROR, ACC, 0x6a, 2)  \
    X(ROR, ZPG, 0x66, 5)  \
    X(ROR, ZPX, 0x76, 6)  \
    X(ROR, ABS, 0x6e, 6)  \
    X(ROR, ABX, 0x7e, 7)  \
    X(RTI, IMP, 0x40, 6)  \
    X(RTS, IMP, 0x60, 6)  \
    X(SBC, IMM, 0xe9, 2)  \
    X(SBC, ZPG, 0xe5, 3)  \
    X(SBC, ZPX, 0xf5, 4)  \
    X(SBC, ABS, 0xed, 4)  \
    X(SBC, ABX, 0xfd, 4)  \
    X(SBC, ABY, 0xf9, 4)  \
    X(SBC, IDX, 0xe1, 6)  \
    X(SBC, IDY, 0xf1, 5)  \
    X(SEC, IMP, 0x38, 2)  \
    X(SED, IMP, 0xf8, 2)  \
    X(SEI, IMP, 0x78, 2)  \
    X(STA, ZPG, 0x85, 3)  \
    X(STA, ZPX, 0x95, 4)  \
    X(STA, ABS, 0x8d, 4)  \
    X(STA, ABX, 0x9d, 5)  \
    X(STA, ABY, 0x99, 5)  \
    X(STA, IDX, 0x81, 6)  \
    X(STA, IDY, 0x91, 6)  \
    X(STX, ZPG, 0x86, 3)  \
    X(STX, ZPY, 0x96, 4)  \
    X(STX, ABS, 0x8e, 4)  \
    X(STY, ZPG, 0x84, 3)  \
    X(STY, ZPX, 0x94, 4)  \
    X(STY, ABS, 0x8c, 4)  \
    X(TAX, IMP, 0xaa, 2)  \
    X(TAY, IMP, 0xa8, 2)  \
    X(TSX, IMP, 0xba, 2)  \
    X(TXA, IMP, 0x8a, 2)  \
    X(TXS, IMP, 0x9a, 2)  \
    X(TYA, IMP, 0x98, 2)

#define X(INSTRUCTION, ADDRESSING_MODE, OPCODE, CYCLES) \
    int INSTRUCTION(CPU *);

INSTRUCTION_SET();

#undef X

/*
    Utilities
*/

// The type of addressing mode for load/store.
enum AddressingModeType
{
    MODE_IMPLIED,     // Implied by instruction
    MODE_ACCUMULATOR, // Accumulator
    MODE_IMMEDIATE,   // Immediate constant
    MODE_ADDRESS,     // Calculated address
};

// Calculate 16-bit address given base and offset (pages wrap)
int getaddress(int base, int offset)
{
    return (base & 0xFF00) | ((base + offset) & 0xFF);
}

// Load the data for an instruction
int load(CPU *cpu, int offset)
{
    switch (cpu->mode)
    {
    case MODE_ACCUMULATOR:
        assert(!offset && "accumulator doesn't support offset");
        return cpu->a;

    case MODE_ADDRESS:
    case MODE_IMMEDIATE:
        return Bus_read(cpu->bus, getaddress(cpu->address, offset));

    default:
        assert(0 && "unexpected load");
        return -1;
    }
}

// Store the data for an instruction
void store(CPU *cpu, int offset, int byte)
{
    switch (cpu->mode)
    {
    case MODE_ACCUMULATOR:
        assert(!offset && "offset should be 0");
        cpu->a = byte;
        return;

    case MODE_ADDRESS:
        Bus_write(cpu->bus, getaddress(cpu->address, offset), byte);
        return;

    default:
        assert(0 && "unexpected store");
        return;
    }
}

// Push a value on to the stack
void push(CPU *cpu, int byte)
{
    Bus_write(cpu->bus, SP_BASE + cpu->sp--, byte);
}

// Push pc on to the stack
void push_pc(CPU *cpu)
{
    // hi first
    push(cpu, cpu->pc >> 8);
    push(cpu, cpu->pc & 0xFF);
}

// Pull a value off of the stack
int pull(CPU *cpu)
{
    return Bus_read(cpu->bus, SP_BASE + ++cpu->sp);
}

// Pull pc off of the stack
void pull_pc(CPU *cpu)
{
    // lo first
    int lo = pull(cpu);
    int hi = pull(cpu);
    cpu->pc = ((hi << 8) | lo);
}

// Convert a int8 to a int
int u8_to_s8(int i)
{
    return (signed char)i;
}

// Execute branch if test was true
int branch(CPU *cpu, int test)
{
    if (test)
    {
        // if we are moving to a new page ++cycle
        if ((cpu->pc & 0xFF00) != (cpu->address & 0xFF00))
        {
            ++cpu->cycles;
        }

        cpu->pc = cpu->address;

        // if the test succeeded ++cycle
        ++cpu->cycles;
    }

    // cycles updated explicitly here. no need for additional cycle logic
    return 0;
}

// Update flags for comparing value with argument
void compare(CPU *cpu, int arg)
{
    int value = load(cpu, 0);
    cpu->c = arg >= value;
    cpu->z = arg == value;
    cpu->n = (arg - value) >> 7;
}

// Update flags for a math result (Z and N)
void set_math_flags(CPU *cpu, int value)
{
    cpu->z = value == 0;
    cpu->n = value >> 7 != 0;
}

// Decrement value by 1 and update flags
int decrement(CPU *cpu, int value)
{
    value = (value - 1) & 0xFF;
    set_math_flags(cpu, value);
    return value;
}

int increment(CPU *cpu, int value)
{
    value = (value + 1) & 0xFF;
    set_math_flags(cpu, value);
    return value;
}

// Trigger interrupt vector after current instruction finished
void interrupt(CPU *cpu, int addr)
{
    // push pc
    push_pc(cpu);

    // disable interrupts
    cpu->i = 1;

    // push status
    PHP(cpu);

    // clear break flag
    cpu->b = 0;

    // load interrupt vector
    int lo = Bus_read(cpu->bus, addr);
    int hi = Bus_read(cpu->bus, addr + 1);
    cpu->pc = (hi << 8) | lo;
}

/*
    Addressing modes

    Set mode and optionally data or address on cpu object.

    Returns 1 if instruction could contain additional cycle.
*/

// Implied
int IMP(CPU *cpu)
{
    cpu->mode = MODE_IMPLIED;
    return 0;
}

// Accumulator
int ACC(CPU *cpu)
{
    cpu->mode = MODE_ACCUMULATOR;
    return 0;
}

// Immediate
int IMM(CPU *cpu)
{
    cpu->mode = MODE_IMMEDIATE;
    cpu->address = cpu->pc++;
    return 0;
}

// Zero Page
int ZPG(CPU *cpu)
{
    cpu->mode = MODE_ADDRESS;
    cpu->address = Bus_read(cpu->bus, cpu->pc++);
    return 0;
}

// Zero Page, X
int ZPX(CPU *cpu)
{
    cpu->mode = MODE_ADDRESS;
    cpu->address = (Bus_read(cpu->bus, cpu->pc++) + cpu->x) & 0xFF;
    return 0;
}

// Zero Page, Y
int ZPY(CPU *cpu)
{
    cpu->mode = MODE_ADDRESS;
    cpu->address = (Bus_read(cpu->bus, cpu->pc++) + cpu->y) & 0xFF;
    return 0;
}

// Relative
int REL(CPU *cpu)
{
    cpu->mode = MODE_ADDRESS;
    cpu->address = cpu->pc + u8_to_s8(Bus_read(cpu->bus, cpu->pc)) + 1;
    ++cpu->pc;
    return 0;
}

// Absolute
int ABS(CPU *cpu)
{
    cpu->mode = MODE_ADDRESS;
    int lo = Bus_read(cpu->bus, cpu->pc++);
    int hi = Bus_read(cpu->bus, cpu->pc++);
    cpu->address = (hi << 8) | lo;
    return 0;
}

// Absolute, X
int ABX(CPU *cpu)
{
    cpu->mode = MODE_ADDRESS;
    int lo = Bus_read(cpu->bus, cpu->pc++);
    int hi = Bus_read(cpu->bus, cpu->pc++);
    cpu->address = ((hi << 8) | lo) + cpu->x;
    return cpu->address >> 8 != hi;
}

// Absolute, Y
int ABY(CPU *cpu)
{
    cpu->mode = MODE_ADDRESS;
    int lo = Bus_read(cpu->bus, cpu->pc++);
    int hi = Bus_read(cpu->bus, cpu->pc++);
    cpu->address = ((hi << 8) | lo) + cpu->y;
    return cpu->address >> 8 != hi;
}

// Indirect
int IND(CPU *cpu)
{
    ABS(cpu); // set cpu->mode and cpu->address
    cpu->address = (load(cpu, 1) << 8) | load(cpu, 0);
    return 0;
}

// Indirect, X
int IDX(CPU *cpu)
{
    ZPX(cpu); // set cpu->mode and cpu->address
    cpu->address = (load(cpu, 1) << 8) | load(cpu, 0);
    return 0;
}

// Indirect, Y
int IDY(CPU *cpu)
{
    ZPG(cpu); // set cpu->mode and cpu->address
    int lo = load(cpu, 0);
    int hi = load(cpu, 1);
    cpu->address = ((hi << 8) | lo) + cpu->y;
    return cpu->address >> 8 != hi;
}

/*
    Instructions

    Returns 1 if instruction could contain additional cycle.
*/

// Add with Carry
int ADC(CPU *cpu)
{
    int input = load(cpu, 0);
    int value = cpu->a + input + cpu->c;
    cpu->c = value > 0xff;
    set_math_flags(cpu, value);
    cpu->v = ((~(cpu->a ^ input) & (cpu->a ^ value)) & 0x0080) != 0;
    cpu->a = value;
    return 1;
}

// Logical AND
int AND(CPU *cpu)
{
    cpu->a &= load(cpu, 0);
    set_math_flags(cpu, cpu->a);
    return 1;
}

// Arithmetic Shift Left
int ASL(CPU *cpu)
{
    int value = load(cpu, 0);
    cpu->c = value >> 7;
    value <<= 1;
    set_math_flags(cpu, value);
    store(cpu, 0, value);
    return 0;
}

// Branch if Carry Clear
int BCC(CPU *cpu)
{
    return branch(cpu, !cpu->c);
}

// Branch if Carry Set
int BCS(CPU *cpu)
{
    return branch(cpu, cpu->c);
}

// Branch if Equal
int BEQ(CPU *cpu)
{
    return branch(cpu, cpu->z);
}

// Bit Test
int BIT(CPU *cpu)
{
    int value = load(cpu, 0);
    cpu->v = (value >> 6) & 1;
    cpu->n = value >> 7;
    value &= cpu->a;
    cpu->z = value == 0;
    return 0;
}

// Branch if Minus
int BMI(CPU *cpu)
{
    return branch(cpu, cpu->n);
}

// Branch if Not Equal
int BNE(CPU *cpu)
{
    return branch(cpu, !cpu->z);
}

// Branch if Positive
int BPL(CPU *cpu)
{
    return branch(cpu, !cpu->n);
}

// Force Break
int BRK(CPU *cpu)
{
    // set break flag
    cpu->b = 1;

    interrupt(cpu, 0xFFFE);

    return 0;
}

// Branch if Overflow Clear
int BVC(CPU *cpu)
{
    return branch(cpu, !cpu->v);
}

// Branch if Overflow Set
int BVS(CPU *cpu)
{
    return branch(cpu, cpu->v);
}

// Clear Carry Flag
int CLC(CPU *cpu)
{
    cpu->c = 0;
    return 0;
}

// Clear Decimal Mode
int CLD(CPU *cpu)
{
    cpu->d = 0;
    return 0;
}

// Clear Interrupt Disable
int CLI(CPU *cpu)
{
    cpu->i = 0;
    return 0;
}

// Clear Overflow Flag
int CLV(CPU *cpu)
{
    cpu->v = 0;
    return 0;
}

// Compare Accumulator
int CMP(CPU *cpu)
{
    compare(cpu, cpu->a);
    return 1;
}

// Compare X Register
int CPX(CPU *cpu)
{
    compare(cpu, cpu->x);
    return 0;
}

// Compare Y Register
int CPY(CPU *cpu)
{
    compare(cpu, cpu->y);
    return 0;
}

// Decrement Memory
int DEC(CPU *cpu)
{
    store(cpu, 0, decrement(cpu, load(cpu, 0)));
    return 0;
}

// Decrement X Register
int DEX(CPU *cpu)
{
    cpu->x = decrement(cpu, cpu->x);
    return 0;
}

// Decrement Y Register
int DEY(CPU *cpu)
{
    cpu->y = decrement(cpu, cpu->y);
    return 0;
}

// Exclusive OR
int EOR(CPU *cpu)
{
    cpu->a ^= load(cpu, 0);
    set_math_flags(cpu, cpu->a);
    return 1;
}

// Increment Memory
int INC(CPU *cpu)
{
    store(cpu, 0, increment(cpu, load(cpu, 0)));
    return 0;
}

// Increment X Register
int INX(CPU *cpu)
{
    cpu->x = increment(cpu, cpu->x);
    return 0;
}

// Increment Y Register
int INY(CPU *cpu)
{
    cpu->y = increment(cpu, cpu->y);
    return 0;
}

// Jump to Address
int JMP(CPU *cpu)
{
    cpu->pc = cpu->address;
    return 0;
}

// Jump to Subroutine
int JSR(CPU *cpu)
{
    push_pc(cpu);
    cpu->pc = cpu->address;
    return 0;
}

// Load Accumulator
int LDA(CPU *cpu)
{
    cpu->a = load(cpu, 0);
    set_math_flags(cpu, cpu->a);
    return 1;
}

// Load X Register
int LDX(CPU *cpu)
{
    cpu->x = load(cpu, 0);
    set_math_flags(cpu, cpu->x);
    return 1;
}

// Load Y Register
int LDY(CPU *cpu)
{
    cpu->y = load(cpu, 0);
    set_math_flags(cpu, cpu->y);
    return 1;
}

// Logical Shift Right
int LSR(CPU *cpu)
{
    int value = load(cpu, 0);
    cpu->c = value & 1;
    value >>= 1;
    set_math_flags(cpu, value);
    store(cpu, 0, value);
    return 0;
}

// No Operation
int NOP(CPU *cpu)
{
    return 0;
}

// Logical Inclusive OR
int ORA(CPU *cpu)
{
    cpu->a |= load(cpu, 0);
    set_math_flags(cpu, cpu->a);
    return 1;
}

// Push Accumulator
int PHA(CPU *cpu)
{
    push(cpu, cpu->a);
    return 0;
}

// Push Processor Status
int PHP(CPU *cpu)
{
    int p = (cpu->n << 7) |
            (cpu->v << 6) |
            (1 << 5) |
            (cpu->b << 4) |
            (cpu->d << 3) |
            (cpu->i << 2) |
            (cpu->z << 1) |
            cpu->c;
    push(cpu, p);
    return 0;
}

// Pull Accumulator
int PLA(CPU *cpu)
{
    cpu->a = pull(cpu);
    set_math_flags(cpu, cpu->a);
    return 0;
}

// Pull Processor Status
int PLP(CPU *cpu)
{
    int p = pull(cpu);
    cpu->n = p >> 7;
    cpu->v = (p >> 6) & 1;
    cpu->b = (p >> 4) & 1;
    cpu->d = (p >> 3) & 1;
    cpu->i = (p >> 2) & 1;
    cpu->z = (p >> 1) & 1;
    cpu->c = p & 1;
    return 0;
}

// Rotate Left
int ROL(CPU *cpu)
{
    int value = load(cpu, 0);
    value <<= 1;
    value |= cpu->c;
    cpu->c = value >> 8;
    value &= 0xff;
    set_math_flags(cpu, value);
    store(cpu, 0, value);
    return 0;
}

// Rotate Right
int ROR(CPU *cpu)
{
    int value = load(cpu, 0);
    value |= cpu->c << 8;
    cpu->c = value & 1;
    value >>= 1;
    set_math_flags(cpu, value);
    store(cpu, 0, value);
    return 0;
}

// Return from Interrupt
int RTI(CPU *cpu)
{
    // pull status
    PLP(cpu);

    // clear break flag
    cpu->b = 0;

    // pull pc
    pull_pc(cpu);

    return 0;
}

// Return from Subroutine
int RTS(CPU *cpu)
{
    pull_pc(cpu);
    ++cpu->pc; // ?
    return 0;
}

// Subtract with Carry
int SBC(CPU *cpu)
{
    int input = load(cpu, 0) ^ 0xFF;
    int value = cpu->a + input + cpu->c;
    cpu->c = value > 0xff;
    set_math_flags(cpu, value);
    cpu->v = (value ^ cpu->a) & (value ^ input) & 0x0080 != 0;
    cpu->a = value;
    return 1;
}

// Set Carry Flag
int SEC(CPU *cpu)
{
    cpu->c = 1;
    return 0;
}

// Set Decimal Flag
int SED(CPU *cpu)
{
    cpu->d = 1;
    return 0;
}

// Set Interrupt Disable
int SEI(CPU *cpu)
{
    cpu->i = 1;
    return 0;
}

// Store Accumulator
int STA(CPU *cpu)
{
    store(cpu, 0, cpu->a);
    return 0;
}

// Store X Register
int STX(CPU *cpu)
{
    store(cpu, 0, cpu->x);
    return 0;
}

// Store Y Register
int STY(CPU *cpu)
{
    store(cpu, 0, cpu->y);
    return 0;
}

// Transfer Accumulator to X
int TAX(CPU *cpu)
{
    cpu->x = cpu->a;
    set_math_flags(cpu, cpu->x);
    return 0;
}

// Transfer Accumulator to Y
int TAY(CPU *cpu)
{
    cpu->y = cpu->a;
    set_math_flags(cpu, cpu->y);
    return 0;
}

//  Transfer Stack Pointer to X
int TSX(CPU *cpu)
{
    cpu->x = cpu->sp;
    set_math_flags(cpu, cpu->x);
    return 0;
}

// Transfer X to Accumulator
int TXA(CPU *cpu)
{
    cpu->a = cpu->x;
    set_math_flags(cpu, cpu->a);
    return 0;
}

//  Transfer X to Stack Pointer
int TXS(CPU *cpu)
{
    cpu->sp = cpu->x;
    return 0;
}

// Transfer Y to Accumulator
int TYA(CPU *cpu)
{
    cpu->a = cpu->y;
    set_math_flags(cpu, cpu->y);
    return 0;
}

/*
    Public functions
*/

void CPU_init(CPU *cpu, Bus *bus)
{
    cpu->bus = bus;
    CPU_reset(cpu);
}

void CPU_reset(CPU *cpu)
{
    // load reset vector from 0xFFFC
    int lo = Bus_read(cpu->bus, 0xFFFC);
    int hi = Bus_read(cpu->bus, 0xFFFD);

    cpu->pc = (hi << 8) | lo;
    cpu->sp = 0xFD; // ?
    cpu->a = 0;
    cpu->x = 0;
    cpu->y = 0;
    cpu->n = 0;
    cpu->v = 0;
    cpu->b = 0;
    cpu->d = 0;
    cpu->i = 0;
    cpu->z = 0;
    cpu->c = 0;

    cpu->cycles = 8;
}

void CPU_irq(CPU *cpu)
{
    // if interupts disabled, do nothing
    if (cpu->i)
    {
        return;
    }

    // clear break flag
    cpu->b = 0;

    interrupt(cpu, 0xFFFE);

    // update cycles
    cpu->cycles += 7;
}

void CPU_nmi(CPU *cpu)
{
    // clear break flag
    cpu->b = 0;

    // after current instruction finished, interrupt
    interrupt(cpu, 0xFFFA);

    // update cycles
    cpu->cycles += 8;
}

#ifndef NDEBUG

void disassemble(CPU *cpu, int addr, int lines)
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
        int lo, hi, op = Bus_read(cpu->bus, addr);
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
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            printf("%04X        " #INSTRUCTION " #$%02X        %02X %02X\n", addr, lo, op, lo);                    \
            addr += 2;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ZPG)                                                                           \
        {                                                                                                          \
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X         %02X %02X\n", addr, lo, op, lo);                    \
            addr += 2;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ZPX)                                                                           \
        {                                                                                                          \
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X,X       %02X %02X\n", addr, lo, op, lo);                    \
            addr += 2;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ZPY)                                                                           \
        {                                                                                                          \
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X,Y       %02X %02X\n", addr, lo, op, lo);                    \
            addr += 2;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == REL)                                                                           \
        {                                                                                                          \
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            printf("%04X        " #INSTRUCTION " $%04X       %02X %02X\n", addr, addr + u8_to_s8(lo) + 2, op, lo); \
            addr += 2;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ABS)                                                                           \
        {                                                                                                          \
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            hi = Bus_read(cpu->bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X%02X       %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ABX)                                                                           \
        {                                                                                                          \
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            hi = Bus_read(cpu->bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X%02X,X     %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == ABY)                                                                           \
        {                                                                                                          \
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            hi = Bus_read(cpu->bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " $%02X%02X,Y     %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == IND)                                                                           \
        {                                                                                                          \
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            hi = Bus_read(cpu->bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " ($%02X%02X)     %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == INX)                                                                           \
        {                                                                                                          \
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            hi = Bus_read(cpu->bus, addr + 2);                                                                     \
            printf("%04X        " #INSTRUCTION " ($%02X%02X,X)   %02X %02X %02X\n", addr, hi, lo, op, lo, hi);     \
            addr += 3;                                                                                             \
        }                                                                                                          \
        else if (ADDRESSING_MODE == INY)                                                                           \
        {                                                                                                          \
            lo = Bus_read(cpu->bus, addr + 1);                                                                     \
            hi = Bus_read(cpu->bus, addr + 2);                                                                     \
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

void print_state(CPU *cpu)
{
    printf("         n v - b d i z c\n");
    printf("status:  %d %d   %d %d %d %d %d\n", cpu->n, cpu->v, cpu->b, cpu->d, cpu->i, cpu->z, cpu->c);
    printf("pc:      $%04X\n", cpu->pc);
    printf("sp:      $1%02X\n", cpu->sp);
    printf("cycles:  %d\n", cpu->cycles);
    printf("          a      x      y\n");
    printf("regs:    #%02X    #%02X    #%02X\n", cpu->a, cpu->x, cpu->y);
    puts("");
    print_memory(cpu->bus, 0, 16);
    puts("");
    disassemble(cpu, cpu->pc, 16);
}

#endif

void CPU_step(CPU *cpu)
{
    if (cpu->cycles)
    {
        --cpu->cycles;
        return;
    }

    int op = Bus_read(cpu->bus, cpu->pc++);
    int am_extra_cycle, in_extra_cycle;

    switch (op)
    {

#define X(INSTRUCTION, ADDRESSING_MODE, OPCODE, CYCLES) \
    case OPCODE:                                        \
        cpu->cycles = CYCLES;                           \
        am_extra_cycle = ADDRESSING_MODE(cpu);          \
        in_extra_cycle = INSTRUCTION(cpu);              \
        break;

        INSTRUCTION_SET();

#undef X

#ifndef NDEBUG
    default:
        printf("Unknown instruction at $%x: $%x\n", cpu->pc, op);
#endif
    }

    // if there is no extra cycle for this instruction, minus 1 already taken
    if ((am_extra_cycle & in_extra_cycle) == 0)
    {
        --cpu->cycles;
    }

#ifndef NDEBUG
    print_state(cpu);
#endif
}
