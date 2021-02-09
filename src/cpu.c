#include <assert.h>

#include "cpu.h"
#include "6502.h"
#include "util.h"

#ifndef NDEBUG
#include <stdio.h>
#endif

#define SP_BASE 0x100

// TODO: decimal?

void CPU_tick(CPU *cpu);

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
    cpu->device.tick = (BusDeviceTick) &CPU_tick;
    cpu->device.read = 0;
    cpu->device.write = 0;

    cpu->bus = bus;
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

void CPU_tick(CPU *cpu)
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
}
