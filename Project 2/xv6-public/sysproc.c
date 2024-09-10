#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  //IMAN S
  //modified so it will check the memory limit before allocating more memory
  if(myproc()->mem_limit > 0 && addr + n > myproc()->mem_limit) // check memory limit
    return -1;
  //IMAN E
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

//IMAN S
int
sys_exec2(void)
{
  char *path, *argv[MAXARG];
  int i, stacksize;
  uint uargv, uarg;

  // Fetching the arguments from the user space
  if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0 || argint(2, &stacksize))
    return -1;
  if(stacksize < 1 || stacksize > 100) // The stack size should be between 1 and 100
    return -1;

  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv))
      return -1;
    if(fetchint(uargv+4*i, (int*)&uarg) < 0)
      return -1;
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    if(fetchstr(uarg, &argv[i]) < 0)
      return -1;
  }
  
  return exec2(path, argv, stacksize);
}
//IMAN E 

//IMAN S
int sys_setmemorylimit(void)
{
  int pid, limit;

  if (argint(0, &pid) < 0 || argint(1, &limit) < 0)
    return -1;

  return setmemorylimit(pid, limit);
}
//IMAN E

//IMAN S
int sys_ps(void)
{
  struct proc_stat* stat;
  int size;

  if(argptr(0, (void*)&stat, sizeof(stat)) < 0 || argint(1, &size) < 0)
    return -1;

  return ps(stat, size);
}
//IMAN E

//IMAN S
int
sys_thread_create(void)
{
  thread_t *thread;
  void *(*start_routine)(void*);
  void *arg;
  
  // Argument extraction
  if(argptr(0, (char**)&thread, sizeof(thread)) < 0 ||
     argptr(1, (char**)&start_routine, sizeof(start_routine)) < 0 ||
     argptr(2, (char**)&arg, sizeof(arg)) < 0)
    return -1;
  
  // Call the real function
  return thread_create(thread, start_routine, arg);
}

int
sys_thread_exit(void)
{
  void *retval;

  // Argument extraction
  if(argptr(0, (char**)&retval, sizeof(retval)) < 0)
    return -1;

  // Call the real function
  thread_exit(retval);

  return 0; // thread_exit() does not return, so this is not reached
}


int
sys_thread_join(void)
{
  thread_t thread;
  void **retval;

  // Argument extraction
  if(argint(0, (int*)&thread) < 0 || argptr(1, (char**)&retval, sizeof(retval)) < 0)
    return -1;

  // Call the real function
  return thread_join(thread, retval);
}

//IMAN E


