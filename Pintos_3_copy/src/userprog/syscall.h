#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <threads/thread.h>
#include <filesys/file.h>
#include <list.h>

void exit_process(char * name, int status);

struct fd_process {
    int fd;
    struct file * f;
    struct list_elem fd_pr_elem;
};

void process_close_fds(int);

void syscall_init (void);

#endif /* userprog/syscall.h */
