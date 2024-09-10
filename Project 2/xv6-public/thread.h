#ifndef _THREAD_H_
#define _THREAD_H_

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"

#define MAX_STACK_SIZE 4000
#define NTHREAD 16

typedef uint thread_t;

// The registers xv6 will save and restore
// to stop and subsequently restart a thread
struct context {
  int eip;
  int esp;
  int ebx;
  int ecx;
  int edx;
  int esi;
  int edi;
  int ebp;
};

// The information a thread needs to execute
struct thread {
  char *kstack;                // Bottom of kernel stack for this thread
  enum threadstate state;      // Thread state
  int pid;                     // Process ID
  struct thread *parent;       // Parent thread
  struct trapframe *tf;        // Trap frame for the current interrupt
  struct context *context;     // Switch here to run thread
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Thread name
  void *(*start_routine)(void*);  // Thread function
  void *arg;                     // Thread function argument
  struct spinlock lock;         // protects this thread structure
  thread_t tid;                 // Thread ID
  int is_thread;                // Flag indicating whether the proc is a thread or not
};

#endif
