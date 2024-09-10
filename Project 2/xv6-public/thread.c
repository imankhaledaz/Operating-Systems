#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "thread.h"
#include "spinlock.h"

extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

extern struct proc *curproc;




typedef int thread_t;
static int nexttid = 1; // Start from 1 as 0 often denotes an error or invalid.

void
start_thread(void *arg1, void (*func)(void*), void *arg2)
{
    func(arg2);
    thread_exit(0);
}


int
thread_create(thread_t *thread, void *(*start_routine)(void*), void *arg)
{
  struct thread *t;
  int i;
  struct proc *curproc = myproc();

  // Allocate thread
  if((t = alloc_thread()) == 0){
    return -1;
  }

  *thread = t->tid;

  if((t->kstack = kalloc()) == 0){
    kfree(t->kstack);
    t->kstack = 0;
    t->state = UNUSED;
    release(&t->lock);
    return -1;
  }
  // Leave room for trap frame.
  t->tf = (struct trapframe*)(t->kstack + KSTACKSIZE) - 1;
  t->tf->eflags = FL_IF;
  t->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  t->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  t->tf->es = t->tf->ds;
  t->tf->ss = t->tf->ds;
  t->tf->esp = USERTOP;
  t->tf->eip = (uint)start_thread;

  // Save function and argument for the new thread
  t->start_routine = start_routine;
  t->arg = arg;

  // Clear %eax so that fork return 0 in the child
  t->tf->eax = 0;

  safestrcpy(t->name, curproc->name, sizeof(curproc->name));
  t->cwd = idup(curproc->cwd);

  // Add thread to process's thread array
  for(i = 0; i < NTHREAD; i++){
    if(curproc->threads[i] == 0){
      curproc->threads[i] = t;
      break;
    }
  }

  acquire(&t->lock);

  // Thread is now ready to run
  t->state = RUNNABLE;

  release(&t->lock);

  return 0;
}


void 
thread_exit(void)
{
  struct thread *curthread = mythread();
  struct proc *curproc = myproc();
  int fd, i;

  if(curproc == 0)
    panic("No Process running");

  if(curthread == 0) 
    panic("No Thread running");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curthread->ofile[fd]){
      fileclose(curthread->ofile[fd]);
      curthread->ofile[fd] = 0;
    }
  }

  iput(curthread->cwd);
  curthread->cwd = 0;

  acquire(&curthread->lock);

  // Parent might be sleeping in thread_join().
  wakeup1(curthread->parent);

  // Jump into the scheduler, never to return.
  curthread->state = ZOMBIE;
  sched();
  panic("zombie thread_exit");
}

void 
thread_exit(void)
{
  struct thread *curthread = mythread();
  struct proc *curproc = myproc();
  int fd, i;

  if(curproc == 0)
    panic("No Process running");

  if(curthread == 0) 
    panic("No Thread running");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curthread->ofile[fd]){
      fileclose(curthread->ofile[fd]);
      curthread->ofile[fd] = 0;
    }
  }

  iput(curthread->cwd);
  curthread->cwd = 0;

  acquire(&curthread->lock);

  // Parent might be sleeping in thread_join().
  wakeup1(curthread->parent);

  // Jump into the scheduler, never to return.
  curthread->state = ZOMBIE;
  sched();
  panic("zombie thread_exit");
}


int
thread_join(thread_t thread)
{
    acquire(&ptable.lock);

    // Look up the thread in the process's thread table
    struct thread *t = 0;
    for(int i = 0; i < NTHREAD; i++) {
        if (curproc->threads[i] && curproc->threads[i]->tid == thread) {
            t = curproc->threads[i];
            break;
        }
    }

    if (t == 0) {
        release(&ptable.lock);
        return -1; // The thread does not exist or has already been waited on
    }

    while (t->state != ZOMBIE) {
        sleep(t, &ptable.lock);
    }

    // The thread is now a zombie. Clean it up.
    kfree(t->kstack);
    t->kstack = 0;
    t->state = UNUSED;
    t->tid = 0;
    t->parent = 0;

    release(&ptable.lock);
    return 0;
}






