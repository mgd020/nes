#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct Header {
    char magic[4]; // 4E 45 53 1A
    unsigned prg_rom : 8; // number of 16KB banks
    unsigned chr_rom : 8; // number of 8KB banks (0 means board uses CHR RAM)
    unsigned flags_6 : 8;
    unsigned flags_7 : 8;
    unsigned prg_ram : 8; // number of 8KB banks, 0 means 1
    unsigned flags_9 : 8;
    unsigned flags_10 : 8;
    char padding[5];
} Header;

enum Flags6 {
    FLAGS_6_MIRRORING = 1 << 0,
    FLAGS_6_RAM_BATTERY = 1 << 1, // battery backed
    FLAGS_6_TRAINER_PRESENT = 1 << 2, // 512B trainer at $7000-$71FF
    FLAGS_6_IGNORE_MIRRORING = 1 << 3, // instead provide four-screen VRAM
    FLAGS_6_MAPPER_LO_NIBBLE = 0XF << 4,
};

enum Mirroring {
    MIRRORING_HORIZONTAL = 0,
    MIRRORING_VERTICAL = 1,
};

enum Flags7 {
    FLAGS_7_VS_UNISYSTEM = 1 << 0,
    FLAGS_7_PLAYCHOICE_10 = 1 << 1,
    FLAGS_7_NES_20 = 3 << 2,
    FLAGS_7_MAPPER_HI_NIBBLE = 0XF << 4,
};

enum Flags9 {
    FLAGS_9_TV_SYSTEM = 1 << 0,
};

enum TVSystem {
    TV_SYSTEM_NTSC = 0,
    TV_SYSTEM_PAL = 1,
};

enum Flags10 {
    FLAGS_10_TV_SYSTEM_DUAL_COMPAT = 1 << 0,
    FLAGS_10_TV_SYSTEM_PAL = 1 << 1, // if off then NTSC, unless DUAL_COMPAT set
    FLAGS_10_PRG_RAM_MISSING = 1 << 4, // $6000-$7FFF, 0: present, 1: missing
    FLAGS_10_BUS_CONFLICTS = 1 << 5,
};

int main() {
    int fd = open("test/nestest.nes", O_RDONLY);
    assert(fd >= 0);
    void *ptr = mmap(0, sizeof(Header), PROT_READ, MAP_PRIVATE, fd, 0);
    assert(ptr != MAP_FAILED);

    Header *header = ptr;
    printf("magic:              %x %x %x %x\n", header->magic[0], header->magic[1], header->magic[2], header->magic[3]);
    printf("prg rom:            %d KB\n", header->prg_rom * 16);
    printf("chr rom:            %d KB\n", header->chr_rom * 8);
    printf("mapper:             %d\n", ((header->flags_6 & FLAGS_6_MAPPER_LO_NIBBLE) >> 4) | (header->flags_7 & FLAGS_7_MAPPER_HI_NIBBLE));
    printf("flags 6:            %s\n", isprint(header->flags_6) ? (char[]){(char)header->flags_6, '\0'} : "");
    printf(" mirroring:         %s\n", (header->flags_6 & FLAGS_6_MIRRORING) ? "vertical" : "horizontal");
    printf(" ram battery:       %d\n", (header->flags_6 & FLAGS_6_RAM_BATTERY) > 0);
    printf(" trainer present:   %d\n", (header->flags_6 & FLAGS_6_TRAINER_PRESENT) > 0);
    printf(" ignore mirroring:  %d\n", (header->flags_6 & FLAGS_6_IGNORE_MIRRORING) > 0);
    printf(" mapper (lo):       %x\n", (header->flags_6 & FLAGS_6_MAPPER_LO_NIBBLE) >> 4);
    printf("flags 7:            %s\n", isprint(header->flags_7) ? (char[]){(char)header->flags_7, '\0'} : "");
    printf(" vs unisystem:      %d\n", (header->flags_7 & FLAGS_7_VS_UNISYSTEM) > 0);
    printf(" playchoice 10:     %d\n", (header->flags_7 & FLAGS_7_PLAYCHOICE_10) > 0);
    printf(" nes 2.0:           %d\n", (header->flags_7 & FLAGS_7_NES_20) >> 2);
    printf(" mapper (hi):       %x\n", (header->flags_7 & FLAGS_7_MAPPER_HI_NIBBLE) >> 4);
    printf("prg ram:            %d KB\n", (header->prg_ram || 1) * 8);
    printf("flags 9:            %s\n", isprint(header->flags_9) ? (char[]){(char)header->flags_9, '\0'} : "");
    printf(" tv system:         %s\n", (header->flags_9 & FLAGS_9_TV_SYSTEM) ? "PAL" : "NTSC");
    printf("flags 10:           %s\n", isprint(header->flags_10) ? (char[]){(char)header->flags_10, '\0'} : "");
    printf(" tv system (dual):  %d\n", (header->flags_10 & FLAGS_10_TV_SYSTEM_DUAL_COMPAT) > 0);
    printf(" tv system:         %s\n", (header->flags_10 & FLAGS_10_TV_SYSTEM_PAL) ? "PAL" : "NTSC");
    printf(" prg ram:           %s\n", (header->flags_10 & FLAGS_10_PRG_RAM_MISSING) ? "missing" : "present");
    printf(" bus conflicts:     %d\n", (header->flags_10 & FLAGS_10_BUS_CONFLICTS) > 0);

    munmap(ptr, sizeof(Header));
    close(fd);
    return 0;
}
