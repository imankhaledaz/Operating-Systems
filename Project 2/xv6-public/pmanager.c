#include "types.h"
#include "user.h"
#include "stat.h"
#include "param.h"
#include "proc.h"

#define MAX_COMMAND_LENGTH 128
#define MAX_ARG_LENGTH 64

// Function prototypes
void list_processes();
void kill_process(int pid);
void execute_process(char* path);
void set_memory_limit(int pid, int limit);

int
main(void) {
  char command[MAX_COMMAND_LENGTH];

  // Main loop
  while (1) {
    printf(1, "pmanager> ");  // Print prompt

    // Read command
    gets(command, sizeof(command));
    command[strlen(command) - 1] = 0;  // Remove newline

    // Handle command
    if (strcmp(command, "list") == 0) {
      list_processes();
    } else if (strncmp(command, "kill ", 5) == 0) {
      int pid = atoi(command + 5);
      kill_process(pid);
    } else if (strncmp(command, "exec ", 5) == 0) {
    char* space = strchr(command + 5, ' ');
    if (space) {
      *space = 0;
      char* path = command + 5;
      int stacksize = atoi(space + 1);
      execute_process(path, stacksize);
    }
    } else if (strncmp(command, "memlim ", 7) == 0) {
      char* space = strchr(command + 7, ' ');
      if (space) {
        *space = 0;
        int pid = atoi(command + 7);
        int limit = atoi(space + 1);
        set_memory_limit(pid, limit);
      }
    } else if (strcmp(command, "quit") == 0) {
      exit();
    } else {
      printf(2, "Invalid command\n");
    }
  }

  exit();
}


// List all processes
// Use ps system call
void list_processes() {
  struct proc_stat stats[NPROC];

  int num_procs = ps(stats, NPROC);

  for (int i = 0; i < num_procs; i++) {
    printf(1, "pid: %d, name: %s, pages: %d, memsize: %d, memlimit: %d\n", 
           stats[i].pid, stats[i].name, stats[i].num_pages, stats[i].memsize, stats[i].memlimit);
  }
}

// Kill a process with given pid
// Use kill system call
void kill_process(int pid) {
  if (kill(pid) < 0) {
    printf(1, "Failed to kill process %d\n", pid);
  }
}


// Execute a process from the given path
// Use fork and exec system calls
void execute_process(char* path, int stacksize) {
  int pid = fork();
  if (pid < 0) {
    printf(1, "Failed to fork process\n");
  } else if (pid == 0) {
    // In child process

    // Pre-allocate stack space by expanding the process's memory
    sbrk(stacksize * PGSIZE);

    if (exec(path, (char* const[]){path, 0}) < 0) {
      printf(1, "Failed to execute %s\n", path);
      exit();
    }
  }
  // In parent process, do nothing
}


// Set memory limit for a process with given pid
// Use setmemorylimit system call
void set_memory_limit(int pid, int limit) {
  if (setmemorylimit(pid, limit) < 0) {
    printf(1, "Failed to set memory limit for process %d\n", pid);
  }
}
