.extern start_kernel
.extern clock_set_next_event
.extern task_init
.extern mm_init
.extern setup_vm
.extern setup_vm_final

.section .text.init
.globl _start
_start:
  //#error "Still have unfilled code!"
  // unimplemented
  la sp, _stack
  li   a5, 0x4000
  add  sp, sp, a5
  li   a5, 0xffffffdf80000000
  sub  sp, sp, a5
  call setup_vm
  call relocate
  jal  mm_init
  call setup_vm_final
  jal  task_init

  la   a5, _traps
  csrw   stvec, a5
  csrr  a5, sie
  ori   a5, a5, 0x20
  csrw  sie, a5
  jal   clock_set_next_event
  #csrr  a5, sstatus
  #ori   a5, a5, 0x2
  #csrw  sstatus, a5
  //addi a5, a0, 888
  j start_kernel 


relocate:
 # set ra = ra + PA2VA_OFFSET
 # set sp = sp + PA2VA_OFFSET (If you have set the sp before)
 li   a5, 0xffffffdf80000000
 add  ra, ra, a5
 add  sp, sp, a5

# set satp with early_pgtbl‘s physical address
 la   t0, early_pgtbl
 sub  t0, t0, a5
 srli t0, t0, 12
 li   a6, 0x8000000000000000
 or   t0, t0, a6
 csrw satp, t0

 sfence.vma zero, zero

 ret