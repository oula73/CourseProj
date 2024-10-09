// clock.c
#include "sbi.h"


unsigned long TIMECLOCK = 10000000;

unsigned long get_cycles() {
    //#error "Still have unfilled code!"
    unsigned long cycles;
    __asm__ volatile("rdtime %[t]" : [t] "=r" (cycles) : : "memory");
    return cycles;
}

void clock_set_next_event() {
    //#error "Still have unfilled code!"
    unsigned long next = get_cycles() + TIMECLOCK;
    sbi_ecall(0x0, 0x0, next, 0, 0, 0, 0, 0);
} 