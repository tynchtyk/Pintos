#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
struct lock block_for_thread;
void syscall_init (void);
void exit (int status);
#endif /* userprog/syscall.h */
