#include "print.h"
#include "sbi.h"
#include "printk.h"
#include "proc.h"
extern char _srodata[];
extern char _stext[];

extern void test();

int start_kernel() {
    // int value;
    // __asm__ volatile(
    //   "mv a5, %[ret]"
    //   : [ret] "=r" (value)
    // );
    // puti(value);
    // puts("\n");
    puti(2022);
    puts(" ZJU Computer System II\n");

    // printk("text = %d\n", *_stext);
    // printk("rodata = %d\n", *_srodata);

    // *_srodata = 5;
    // printk("hh\n");

    schedule();
    test(); // DO NOT DELETE !!!

	return 0;
}
