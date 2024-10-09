#include "proc.h"
#include "defs.h"
#include "printk.h"
#include "mm.h"
#include "rand.h"
//arch/riscv/kernel/proc.c

extern void __dummy();
extern void __switch_to(struct task_struct* prev, struct task_struct* next);
extern void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);
extern void ret_from_fork(struct pt_regs *trapframe);
extern unsigned long  swapper_pg_dir[];
extern char uapp_start[];
extern char uapp_end[];

struct task_struct* idle;           // idle process
struct task_struct* current;        // 指向当前运行线程的 `task_struct`
struct task_struct* task[NR_TASKS]; // 线程数组，所有的线程都保存在此

int total_tasks;

void task_init() {
    // 1. 调用 kalloc() 为 idle 分配一个物理页
    // 2. 设置 state 为 TASK_RUNNING;
    // 3. 由于 idle 不参与调度 可以将其 counter / priority 设置为 0
    // 4. 设置 idle 的 pid 为 0
    // 5. 将 current 和 task[0] 指向 idle

    /* YOUR CODE HERE */
    task[0] = (struct task_struct*)kalloc();
    task[0]->state = TASK_RUNNING;
    task[0]->counter = 0;
    task[0]->priority = 0;
    task[0]->pid = 0;
    current = task[0];
    idle = task[0];
    //printk("current = %x  current_addr = %x\n",current, &current);

    // 1. 参考 idle 的设置, 为 task[1] ~ task[NR_TASKS - 1] 进行初始化
    // 2. 其中每个线程的 state 为 TASK_RUNNING, counter 为 0, priority 使用 rand() 来设置, pid 为该线程在线程数组中的下标。
    // 3. 为 task[1] ~ task[NR_TASKS - 1] 设置 `thread_struct` 中的 `ra` 和 `sp`, 
    // 4. 其中 `ra` 设置为 __dummy （见 4.3.2）的地址， `sp` 设置为 该线程申请的物理页的高地址

    /* YOUR CODE HERE */
    int i;
    total_tasks = 2;
    //for (i = 1;i < NR_TASKS; i++)
    for(i = 1; i < 2; i++)
    {
        uint64 page = kalloc();
        task[i] = (struct task_struct*)page;
        task[i]->state = TASK_RUNNING;
        task[i]->counter = 0;
        //task[i]->priority = rand();
        task[i]->priority = i;
        task[i]->pid = i;
        task[i]->thread.ra = (uint64)__dummy;
        task[i]->thread.sp = page + PGSIZE;

        //if (i==3) printk("page=%x  info=%x thread=%x ra=%x sp=%x s[0]=%x\n",page,&(task[3]->thread_info),&(task[3]->thread),&(task[3]->thread.ra),&(task[3]->thread.sp),&(task[3]->thread.s[0]));
        
        pagetable_t pgt = (pagetable_t)kalloc();
        task[i]->pgd = (pagetable_t)((((uint64)pgt - PA2VA_OFFSET) >> 12) | 0x8000000000000000);
        //printk("satp = %lx, va = %lx\n", task[i]->pgd, pgt);

        int j;
        for (j = 0; j < 512; j++)
        {
            pgt[j] = swapper_pg_dir[j];
        }
        //printk("hh\n");
        task[i]->mm = (struct mm_struct*)kalloc();
        task[i]->mm->mmap = NULL;

        //uint64 U_Stack = kalloc();
        do_mmap(task[i]->mm, (uint64)USER_START, (uint64)(uapp_end - uapp_start), 31);
        //create_mapping(pgt, (uint64)USER_START, (uint64)uapp_start - PA2VA_OFFSET, (uint64)(uapp_end - uapp_start), 31);
        do_mmap(task[i]->mm, (uint64)(USER_END - PGSIZE), (uint64)PGSIZE, 23);
        //create_mapping(pgt, (uint64)(USER_END - PGSIZE), (uint64)U_Stack - PA2VA_OFFSET, (uint64)PGSIZE, 23);
        struct vm_area_struct* p = task[i]->mm->mmap;
        // while(p)
        // {
        //     printk("start = %lx, end = %lx, flags = %lx\n", p->vm_start, p->vm_end, p->vm_flags);
        //     p = p->vm_next;
        // }



        /*
        FOR DEBUG


        //printk("user_pgt = %lx\n", pgt);
        //printk("stack_start = %lx, stack_end = %lx\n", USER_END - PGSIZE, USER_END - 1);
        //printk("2 done\n");
        // printk("satp = %lx\n", task[i]->pgd);
        // printk("PTE[0] = %lx, va = %lx\n",pgt[0], (uint64)((pgt[0] >> 10) << 12) + PA2VA_OFFSET);
        // uint64* va1 = (uint64*)((uint64)((pgt[0] >> 10) << 12) + PA2VA_OFFSET);
        // printk("PTE[1] = %lx, va = %lx\n", va1[0], (uint64)((va1[0] >> 10) << 12) + PA2VA_OFFSET);
        // uint64* va2 = (uint64*)((uint64)((va1[0] >> 10) << 12) + PA2VA_OFFSET);
        // printk("PTE[2] = %lx, va = %lx, final = %lx\n", va2[0], (uint64)((va2[0] >> 10) << 12) + PA2VA_OFFSET, (uint64)((va2[0] >> 10) << 12));
        // printk("uapp_start_pa = %lx\n", (uint64)uapp_start - PA2VA_OFFSET);
        //printk("uapp_start = %lx, uapp_end = %lx\n", uapp_start, uapp_end);


        FOR DEBUG
        */


        task[i]->thread.sepc = (uint64)USER_START;
        uint64 status;
        __asm__ volatile(
            "csrr %0, sstatus" 
            : "=r" (status)
        );
        status = status & (~(1 << 8));   // SPP  8
        status = status | 0x40020;       // SPIE 5  SUM 18
        task[i]->thread.sstatus = status;
        task[i]->thread.sscratch = (uint64)USER_END;
        //printk("user_end = %lx\n", (uint64)USER_END);
        //printk("hh\n");
    }
    

    printk("...proc_init done!\n");
}

void forkret() 
{
    ret_from_fork(current->trapframe);
}

uint64 do_fork(struct pt_regs *regs) 
{
    int i = total_tasks++;

    uint64 page = kalloc();
    task[i] = (struct task_struct*)page;
    task[i]->thread_info = (struct thread_info*)kalloc(); 
    task[i]->state = TASK_RUNNING;
    task[i]->counter = 0;
    task[i]->priority = i;
    task[i]->pid = i;

    uint64* U_Stack = (uint64*)kalloc();
    uint64* U_Stack_P = (uint64*)(USER_END - PGSIZE);
    task[i]->thread_info->user_sp = (uint64)U_Stack;
    int j;
    for(j = 0; j < 512; j++)
    {
        U_Stack[j] = U_Stack_P[j];
    }

    task[i]->thread.ra = (uint64)forkret;
    task[i]->thread.sp = page + PGSIZE;
    task[i]->thread.sscratch = page + PGSIZE;
    task[i]->thread.sepc = regs->sepc + 4;
    task[i]->thread.sstatus = regs->sstatus;

    pagetable_t pgt = (pagetable_t)kalloc();
    task[i]->pgd = (pagetable_t)((((uint64)pgt - PA2VA_OFFSET) >> 12) | 0x8000000000000000);

    for (j = 0; j < 512; j++)
    {
        pgt[j] = swapper_pg_dir[j];
    }
    //printk("pgt[0] = %lx\n", pgt[0]);
    //printk("pgt = %lx\n", pgt);
    //printk("satp = %lx\n", task[i]->pgd);

    task[i]->mm = (struct mm_struct*)kalloc();
    //task[i]->mm->mmap = current->mm->mmap;
    for (struct vm_area_struct* p = current->mm->mmap; p != NULL; p = p->vm_next)
        do_mmap(task[i]->mm, p->vm_start, p->vm_end - p->vm_start, p->vm_flags);

    task[i]->trapframe = (struct pt_regs*)kalloc();
    
    task[i]->trapframe->reg[0] = regs->reg[0];
    task[i]->trapframe->reg[1] = regs->reg[1];
    //task[i]->trap_frame->reg[2] = regs->reg[2];
    //sp = sscratch (t2)
    task[i]->trapframe->reg[2] = regs->reg[7];
    task[i]->trapframe->reg[3] = regs->reg[3];
    task[i]->trapframe->reg[4] = regs->reg[4];
    task[i]->trapframe->reg[5] = regs->reg[5];
    task[i]->trapframe->reg[6] = regs->reg[6];
    //sscratch = sp + 280
    //task[i]->trapframe->reg[7] = regs->reg[2] + 280;
    task[i]->trapframe->reg[7] = page + PGSIZE;
    task[i]->trapframe->reg[8] = regs->reg[8];
    task[i]->trapframe->reg[9] = regs->reg[9];
    //task[i]->trap_frame->reg[10] = regs->reg[10];
    // a0 = 0
    task[i]->trapframe->reg[10] = 0;
    task[i]->trapframe->reg[11] = regs->reg[11];
    task[i]->trapframe->reg[12] = regs->reg[12];
    task[i]->trapframe->reg[13] = regs->reg[13];
    task[i]->trapframe->reg[14] = regs->reg[14];
    task[i]->trapframe->reg[15] = regs->reg[15];
    task[i]->trapframe->reg[16] = regs->reg[16];
    task[i]->trapframe->reg[17] = regs->reg[17];
    task[i]->trapframe->reg[18] = regs->reg[18];
    task[i]->trapframe->reg[19] = regs->reg[19];
    task[i]->trapframe->reg[20] = regs->reg[20];
    task[i]->trapframe->reg[21] = regs->reg[21];
    task[i]->trapframe->reg[22] = regs->reg[22];
    task[i]->trapframe->reg[23] = regs->reg[23];
    task[i]->trapframe->reg[24] = regs->reg[24];
    task[i]->trapframe->reg[25] = regs->reg[25];
    task[i]->trapframe->reg[26] = regs->reg[26];
    task[i]->trapframe->reg[27] = regs->reg[27];
    task[i]->trapframe->reg[28] = regs->reg[28];
    task[i]->trapframe->reg[29] = regs->reg[29];
    task[i]->trapframe->reg[30] = regs->reg[30];
    task[i]->trapframe->reg[31] = regs->reg[31];
    task[i]->trapframe->sepc = regs->sepc + 4;
    task[i]->trapframe->sstatus = regs->sstatus;
    task[i]->trapframe->stval = regs->stval;

    //printk("[fork] sepc = %lx, task[%d] = %lx, trapframe=%lx\n", task[i]->thread.sepc, i, task[i], task[i]->trapframe);

    return i;
}

uint64 clone(struct pt_regs *regs) 
{
    return do_fork(regs);
}

struct vm_area_struct *find_vma(struct mm_struct *mm, uint64 addr)
{
    if (!mm) return NULL;
    struct vm_area_struct* p = mm->mmap;

    while(p)
    {
        if (addr >= p->vm_start && addr < p->vm_end)
            break;
        p = p->vm_next;
    }

    return p;
}

uint64 do_mmap(struct mm_struct *mm, uint64 addr, uint64 length, int prot)
{
    if (!mm) return -1;
    struct vm_area_struct* p = mm->mmap;
    struct vm_area_struct* q = NULL;
    struct vm_area_struct* end = NULL;
    uint64 addr_end = addr + length;
    //bool flag = true;
    int flag = 1;
    //bool first = (p == NULL);
    int first = 0;
    if (p == NULL) first = 1;

    struct vm_area_struct* vm_unit = (struct vm_area_struct*)kalloc();
    vm_unit->vm_mm = mm;
    vm_unit->vm_flags = prot;
    vm_unit->vm_prev = NULL;
    vm_unit->vm_next = NULL;

    while(p)
    {
        if (p->vm_start > addr && !q)
            q = p;
        if (p->vm_end <= addr || p->vm_start >= addr_end)
        {
            if (p->vm_next == NULL)
                end = p;
            p = p->vm_next;
            continue;
        }
        flag = 0;
        break;
    }

    if (flag == 0)
    {
        addr = get_unmapped_area(mm, length);
        addr_end = addr + length;
        p = mm->mmap;
        q = NULL;
        while(p)
        {
            if (p->vm_start > addr)
            {
                q = p;
                break;
            }
            if (p->vm_next == NULL)
                end = p;
            p = p->vm_next;
        }
    }
    
    vm_unit->vm_start = addr;
    vm_unit->vm_end = addr_end;
    vm_unit->vm_next = q;
    if (!q) 
    {
        vm_unit->vm_prev = end;
        if (end) end->vm_next = vm_unit;
    }
    else if (q == mm->mmap) 
    {
        mm->mmap = vm_unit;
        q->vm_prev = vm_unit;
    }
    else 
    {
        vm_unit->vm_prev = q->vm_prev;
        q->vm_prev = vm_unit;
    }

    if (first) mm->mmap = vm_unit;

    return addr;
}

uint64 get_unmapped_area(struct mm_struct *mm, uint64 length)
{
    uint64 va = 0;
    uint64 va_end = va + length;
    struct vm_area_struct* p = mm->mmap;

    while(p)
    {
        if (p->vm_end <= va || p->vm_start >= va_end)
        {
            p = p -> vm_next;
            continue;
        }
        va = PGROUNDUP(va_end);
    }

    return va;
}


void dummy() {
    uint64 MOD = 1000000007;
    uint64 auto_inc_local_var = 0;
    int last_counter = -1; // 记录上一个counter
    int last_last_counter = -1; // 记录上上个counter
    while(1) {
        if (last_counter == -1 || current->counter != last_counter) {
            last_last_counter = last_counter;
            last_counter = current->counter;
            auto_inc_local_var = (auto_inc_local_var + 1) % MOD;
            //printk("[PID = %d] is running. auto_inc_local_var = %d\n", current->pid, auto_inc_local_var);
             printk("[PID = %d] is running. Thread space begin at %lx\n", current->pid, current);
             //printk("Thread variable is at %lx\n", &last_counter);
        } else if((last_last_counter == 0 || last_last_counter == -1) && last_counter == 1) { // counter恒为1的情况
            // 这里比较 tricky，不要求理解。
            last_counter = 0;
            current->counter = 0;
    }
    }
}


void switch_to(struct task_struct* next) {
    /* YOUR CODE HERE */

    if (current == next) return ;
    else 
    {
        //printk("hhh\n");
        struct task_struct *previous = current;
        current = next;
        __switch_to(previous, next);
    }
}


void do_timer(void) {
    /* 1. 将当前进程的counter--，如果结果大于零则直接返回*/
    /* 2. 否则进行进程调度 */

    /* YOUR CODE HERE */
    //printk("in do_timer: current = %x  current_addr = %x\n",current, &current);
    if (!current) return ;
    current->counter -= 1;
    if ((int)(current->counter) > 0) return ;
    else schedule();
}

void schedule(void) {
    /* YOUR CODE HERE */
    int i,c,index;
    struct task_struct *next;

    while (1)
    {
        c = 11;
        next = 0;
        for (i = 1;i < total_tasks;i++)
        {
            if (task[i]->counter > 0 && task[i]->counter < c)
            {
                c = task[i]->counter;
                next = task[i];
                index = i;
            }
        }
        if (c < 11) break;
        for (i = 1;i < total_tasks;i++)
        {
            //task[i]->counter = task[i]->priority;
            task[i]->counter = i;
            printk("SET [PID = %d, PRIORITY = %d, COUNTER = %d]\n", i, task[i]->priority, task[i]->counter);
        }
    }
    printk("switch to [PID = %d, PRIORITY = %d, COUNTER = %d]\n",index, next->priority, next->counter);
    switch_to(next);
}