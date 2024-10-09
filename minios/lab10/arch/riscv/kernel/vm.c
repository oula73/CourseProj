#include "defs.h"
#include "types.h"
#include "mm.h"
#include "string.h"
#include "printk.h"

extern char _stext[];
extern char _srodata[];
extern char _etext[];
extern char _erodata[];
extern char _sdata[];
extern char _edata[];

// arch/riscv/kernel/vm.c

/* early_pgtbl: 用于 setup_vm 进行 1GB 的 映射。 */
unsigned long early_pgtbl[512] __attribute__((__aligned__(0x1000)));

void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm);

void setup_vm(void)
{
    /*
    1. 由于是进行 1GB 的映射 这里不需要使用多级页表
    2. 将 va 的 64bit 作为如下划分： | high bit | 9 bit | 30 bit |
        high bit 可以忽略
        中间9 bit 作为 early_pgtbl 的 index
        低 30 bit 作为 页内偏移 这里注意到 30 = 9 + 9 + 12， 即我们只使用根页表， 根页表的每个 entry 都对应 1GB 的区域。
    3. Page Table Entry 的权限 V | R | W | X 位设置为 1
    4. early_pgtbl 对应的是虚拟地址，而在本函数中你需要将其转换为对应的物理地址使用
    */
   //printk("hello!\n");

    uint64 PA = 0x80000000;
    uint64 VA1 = PA;
    uint64 VA2 = 0xffffffe000000000;
    uint64 PPN = PA >> 12;
    int index1 = (VA1 >> 30) & 0x1ff;
    int index2 = (VA2 >> 30) & 0x1ff;

    //uint64* early_pgtbl_pa = (uint64*) ((uint64)early_pgtbl - PA2VA_OFFSET);

    early_pgtbl[index1] = (PPN << 10) | 0xf;
    early_pgtbl[index2] = (PPN << 10) | 0xf;

    // uint64 sa = ((uint64)early_pgtbl - PA2VA_OFFSET) | 0x8000000000000000 ;
    // csr_write(satp, sa);

    //printk("...setup_vm done!\n");
}

// arch/riscv/kernel/vm.c 

/* swapper_pg_dir: kernel pagetable 根目录， 在 setup_vm_final 进行映射。 */
unsigned long  swapper_pg_dir[512] __attribute__((__aligned__(0x1000)));

void setup_vm_final(void) {
    memset(swapper_pg_dir, 0x0, PGSIZE);

    // No OpenSBI mapping required

    // mapping kernel text X|-|R|V
    create_mapping(swapper_pg_dir, (uint64)_stext, (uint64)(_stext - PA2VA_OFFSET), (uint64)(_etext - _stext), 11);

    // mapping kernel rodata -|-|R|V
    create_mapping(swapper_pg_dir, (uint64)_srodata, (uint64)(_srodata - PA2VA_OFFSET), (uint64)(_erodata - _srodata), 3);

    // mapping other memory -|W|R|V
    //create_mapping(swapper_pg_dir, (uint64)_sdata, (uint64)(_sdata - PA2VA_OFFSET), (uint64)(_edata - _sdata), 7);
    create_mapping(swapper_pg_dir, (uint64)_sdata, (uint64)_sdata- PA2VA_OFFSET, PHY_END + PA2VA_OFFSET - (uint64)_sdata, 7);

    // set satp with swapper_pg_dir
    uint64 sa = (((uint64)swapper_pg_dir - PA2VA_OFFSET) >> 12) | 0x8000000000000000 ;
    //uint64 sa = ((uint64)swapper_pg_dir) | 0x8000000000000000 ;
    csr_write(satp, sa);

    //YOUR CODE HERE

    // flush TLB
    asm volatile("sfence.vma zero, zero");
    printk("...setup_vm_final done!\n");
    return;
}


/* 创建多级页表映射关系 */
void create_mapping(uint64 *pgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
    /*
    pgtbl 为根页表的基地址
    va, pa 为需要映射的虚拟地址、物理地址
    sz 为映射的大小
    perm 为映射的读写权限

    将给定的一段虚拟内存映射到物理内存上
    物理内存需要分页
    创建多级页表的时候可以使用 kalloc() 来获取一页作为页表目录
    可以使用 V bit 来判断页表项是否存在
    */
    
    uint64* pgtbl1;
    uint64* pgtbl0;
    uint64 pgtbl_entry;
    uint64 vpn2, vpn1, vpn0;
    uint64 va_end = va + sz;

    //printk("va = %lx, pa = %lx\n", va, pa);


    while (va < va_end) {
        vpn2 = (va >> 30) & 0x1ff;
        //if (perm == 27) printk("vpn2 = %x\n", vpn2);
        
        if (pgtbl[vpn2] & 0x1)  
            pgtbl1 = (uint64*)((((uint64)pgtbl[vpn2] >> 10) << 12));
        else 
        {
            pgtbl1 = (uint64*)((uint64)kalloc() - PA2VA_OFFSET);
            pgtbl[vpn2] = ((((uint64)pgtbl1) >> 12) << 10) | 0x1;
            if (perm == 31 || perm == 23) pgtbl[vpn2] |= 0x10;
        }
        
        vpn1 = (va >> 21) & 0x1ff;
        //if (perm == 27) printk("pgtbl1 = %x\n", pgtbl1);
        //if (perm == 27) printk("pgtbl1 = %x, vpn1 = %x, value = %x\n", pgtbl1, vpn1, pgtbl1[vpn1]);
        if (perm == 31 || perm == 23) 
        {
            pgtbl1 = (uint64*)((uint64)pgtbl1 + PA2VA_OFFSET);
        }

        if (pgtbl1[vpn1] & 0x1) 
        {
            pgtbl0 = (uint64*)((((uint64)pgtbl1[vpn1] >> 10) << 12)); 
        }
        else 
        {
            pgtbl0 = (uint64*)((uint64)kalloc() - PA2VA_OFFSET);
            pgtbl1[vpn1] = ((((uint64)pgtbl0) >> 12) << 10) | 0x1;
            if (perm == 31 || perm == 23) pgtbl1[vpn1] |= 0x10;
        }

        vpn0 = (va >> 12) & 0x1ff;
        //if (perm == 27) printk("vpn0 = %x\n", vpn0);
        if (perm == 31 || perm == 23) 
        {
            pgtbl0 = (uint64*)((uint64)pgtbl0 + PA2VA_OFFSET);
            // printk("pgtbl = %lx, vpn = %lx\n", pgtbl, vpn2);
            // printk("pgtbl1 = %lx, vpn = %lx\n", pgtbl1, vpn1);
            // printk("pgtbl0 = %lx, vpn = %lx\n", pgtbl0, vpn0);
            //printk("gggg\n");
        }

        if (!(pgtbl0[vpn0] & 0x1))
            pgtbl0[vpn0] = ((pa >> 12) << 10) | perm;

        va += PGSIZE;
        pa += PGSIZE;
        // if (perm == 27)
        //     printk("va = %x, va_end = %x\n", va, va_end);
    }
    //printk("gg\n");
}
