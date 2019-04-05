#ifndef PAGES_H
#define PAGES_H

#include "types.h"

#define KILOBYTE 1024 // 1KB
#define FOUR_KB 4096 // 4KB
#define MEGABYTE 0x100000 // 1MB
#define FOUR_MB 0x400000 // 4MB

#define SIZE_F 0x80 // bit [7], 1 = 4MB, 0 = 4KB
#define GLOBAL_F 0x100
#define PRESENT_F 0x1 // bit [0], 1 = present, 0 = not present
#define PRIVILEGE_F 0x10 // bit [2], 1 = user, 0 = supervisor

#define KERNEL 0x400000 // 4MB
#define VIDEO_LOC 0xB8000

extern uint32_t pageDirectory[1024] __attribute__((aligned(4096)));
extern uint32_t pageTable[1024] __attribute__((aligned(4096)));
extern uint32_t pageTableTerminal[1024] __attribute__((aligned(4096)));
extern void init_pages(uint32_t * ptr);

void init_pd();
#endif
