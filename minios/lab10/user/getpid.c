#include "syscall.h"
#include "stdio.h"


static inline long getpid() {
    long ret;
    asm volatile ("li a7, %1\n"
                  "ecall\n"
                  "mv %0, a0\n"
                : "+r" (ret) 
                : "i" (SYS_GETPID));
    return ret;
}

static inline long fork()
{
  long ret;
  asm volatile ("li a7, %1\n"
                "ecall\n"
                "mv %0, a0\n"
                : "+r" (ret) : "i" (SYS_CLONE));
  return ret;
}

// int main() {
//     register unsigned long current_sp __asm__("sp");
//     while (1) {
//         printf("[U-MODE] pid: %ld, sp is %lx\n", getpid(), current_sp);
//         for (unsigned int i = 0; i < 0x13FFFFFF; i++);
//     }
//     return 0;
// }


// int main() {
//     int pid;
//     printf("[U] Enter main\n");
//     pid = fork();
//     printf("[U] pid: %ld\n", pid);

//     if (pid == 0) {
//         while (1) {
//             printf("[U-CHILD] pid: %ld is running!\n", getpid());
//             for (unsigned int i = 0; i < 0x9FFFFFF; i++);
//         } 
//     } else {
//         while (1) {
//             printf("[U-PARENT] pid: %ld is running!\n", getpid());
//             for (unsigned int i = 0; i < 0x9FFFFFF; i++);
//         } 
//     }
//     return 0;
// }

int main() {
  int pid;
  pid = fork();
  if(pid > 0)
    printf("[PID = %d] fork [PID = %d]\n", getpid(), pid);
  pid = fork();
  if(pid > 0)
    printf("[PID = %d] fork [PID = %d]\n", getpid(), pid);

  while(1) {
      printf("[PID = %d] is running!\n", getpid());
      for (unsigned int i = 0; i < 0x13FFFFFF; i++);
      //for (unsigned int i = 0; i < 0x13FFFF; i++);
  }
}
