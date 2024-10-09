// trap.c 
#include "printk.h"
#include "clock.h"
#include "proc.h"
#include "defs.h"
#include "mm.h"

extern void putc(char c);
extern void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);
extern struct task_struct* current;
extern char uapp_start[];

long sys_write(unsigned int fd, const char* buf, size_t count)
{
    if (fd != 1)
    {
        printk("fd != 1, ERROR!\n");
        return -1;
    }
    else 
    {
        int i;
        for (i = 0; i < count; i++)
        {
            putc(buf[i]);
        }
        return 1;
    }
}

long sys_getpid()
{
    return current->pid;
}

void do_page_fault(struct pt_regs *regs, unsigned long scause)
{
    uint64 bad_va = regs->stval;
    struct mm_struct* mm_unit = current->mm;
    struct vm_area_struct* vm_unit = find_vma(current->mm, bad_va);
    if (!vm_unit)
    {
        printk("Illegle VA!\n");
        printk("bad_va = %lx, mm = %lx\n", bad_va, current->mm);
        while(1) ;
        return ;
    }
    //printk("start = %lx, end = %lx\n", vm_unit->vm_start, vm_unit->vm_end);
    printk("[S] PAGE_FAULT: scause: %lx, sepc: %lx, badaddr: %lx\n", scause, regs->sepc, bad_va);
    uint64 va_start = vm_unit->vm_start;
    uint64 va_end = vm_unit->vm_end;
    uint64 va_size = va_end - va_start;
    uint64 flags = vm_unit->vm_flags;
    pagetable_t pgt = (pagetable_t)(((uint64)current->pgd << 12) + PA2VA_OFFSET);
    if (flags == 31)
    {
        //printk("pgd = %lx, va = %lx, pa = %lx\n",pgt, va_start, (uint64)uapp_start - PA2VA_OFFSET);
        create_mapping(pgt, va_start, (uint64)uapp_start - PA2VA_OFFSET, va_size, (int)flags);
    }
    else if (flags == 23)
    {
        uint64 U_Stack;
        if (current->thread_info) U_Stack = current->thread_info->user_sp;
        else U_Stack = kalloc();
        create_mapping(pgt, va_start, U_Stack - PA2VA_OFFSET, va_size, (int)flags);
    }
    else 
    {
        printk("Error Flags!\n");
    }
    //printk("done\n");
}

void trap_handler(unsigned long scause, unsigned long sepc, struct pt_regs *regs) {
    //#error "Still have unfilled code!"
    if (scause == 0x8000000000000005)
    {
        //printk("[S] Supervisor Mode Timer Interrupt\n");
        clock_set_next_event();
        do_timer();
    }
    else if (scause == 0x8)
    {
        long ret_value = -1;
        uint64 exception = regs->reg[17];
        if (exception == 64)
        {
            //printk("fd = %lx, buf = %lx, cnt = %lx\n", regs->reg[10], regs->reg[11], regs->reg[12]);
            ret_value =  sys_write((unsigned int)regs->reg[10], (const char*)regs->reg[11], (size_t)regs->reg[12]);
        }
        else if (exception == 172)
        {
            //printk("hh\n");
            ret_value = sys_getpid();
            //printk("pid = %lx\n", ret_value);
        }
        else if (exception == 220)
        {
            ret_value =  clone(regs);
        }
        else
        {
            printk("exception = %lx, Unknown excpetion code!\n", exception);
            ret_value = -1;
        }
        regs->reg[10] = ret_value;
        regs->sepc += 4;
    }
    else if (scause == 12 || scause == 13 || scause == 15)
    {
        //printk("scause = %lx\n", scause);
        do_page_fault(regs, scause);
    }
    else 
    {
        printk("%lx  ",scause);
        printk("Interrupt type error\n");
    }
}

