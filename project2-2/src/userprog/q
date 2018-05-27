#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/init.h"
#include "devices/shutdown.h" 
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "threads/malloc.h"
#include <stdbool.h>
#include <debug.h>

#define REG (uint32_t)

typedef int pid_t;
void exit (int status);
struct file* get_file(int fd);
static void syscall_handler (struct intr_frame *);
struct descriptor
{
    struct list_elem fe;
    struct file *f;
    int fd;
};

int some_fd = 3;

struct lock block_for_thread;

void
syscall_init (void)
{
  lock_init(&block_for_thread);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED)
{
    int args[3], *p = f -> esp, call = *p, i=0;
    while(i < 3)
    {
      int *ptr = p + i + 1;
      if(!is_user_vaddr( (const void *) ptr) )  exit(-1);
      args[i] = *ptr; 
      i++;
    }
                switch(call)
		{
			case SYS_HALT:
				shutdown_power_off();
				break;
			case SYS_EXIT:
				exit(args[0]);
				break;

			case SYS_CREATE:
        			args[0] = (int) pagedir_get_page(thread_current()->pagedir, (const void *) args[0]);
				if((const char *)args[0] == NULL) 	
					exit(-1);
				lock_acquire(&block_for_thread);
				f->eax = filesys_create((const char *)args[0], (unsigned) args[1]);
				lock_release(&block_for_thread);				
				break;

			case SYS_OPEN:
			        args[0] =(int) pagedir_get_page(thread_current()->pagedir, (const void *) args[0]);
			    	if((const char *)args[0] == NULL) 	
					exit(-1);
				  lock_acquire(&block_for_thread);
				  struct file* file_ = filesys_open((const char *) args[0]);
				  if(file_ == NULL)
					  {
					    lock_release(&block_for_thread);
					    f->eax = REG -1;
					  	break;
						}
				  struct descriptor *desc = malloc(sizeof(struct descriptor));
				  desc->f = file_;
				  desc->fd = some_fd++;

				  list_push_back(&thread_current ()->fd_list, &desc->fe);
				  lock_release(&block_for_thread);
				  f->eax = REG some_fd-1;
             				break;

			case SYS_CLOSE:
                                 if((const char *)args[0] == NULL) 	
					exit(-1);
				 lock_acquire(&block_for_thread);
 				 struct list *seq = &thread_current()->fd_list;
	               		 struct list_elem *i = list_begin(seq);
 				 while(i != NULL)
 				 {
 				     struct descriptor *t = list_entry (i, struct descriptor, fe);
 				     if (args[0] == t->fd)
 				     {
 				       file_close(t->f);
 				       list_remove(&t->fe);
 				     }
 				    i = i->next;
	 			 }
				lock_release(&block_for_thread);
				break;

			case SYS_WRITE:
			        args[1] = (int) pagedir_get_page(thread_current()->pagedir, (const void *) args[1]);
				lock_acquire(&block_for_thread);
				if (args[0] == 0) {
				    lock_release(&block_for_thread);
    				    f->eax = REG 0; break;
                                }
				if(args[0] == 1)  {
					putbuf((const void *) args[1], (unsigned) args[2]);
  					lock_release(&block_for_thread);
					f->eax = REG (unsigned) args[2]; break;
				}
				struct file* F = get_file(args[0]);
				if(F != NULL) {
				        f->eax = REG (int) file_write(F, (const void *) args[1], (unsigned) args[2]);
				        lock_release(&block_for_thread);
				   	break;
				}
  				lock_release(&block_for_thread);
				f->eax = REG 0;
			        break;
			default:
				exit(-1);
				break;
		}
}

struct file* get_file(int fd) { 
	struct list_elem *i = list_begin(&thread_current()->fd_list);  
	while (i != NULL) {
	      	struct descriptor *t = list_entry (i, struct descriptor, fe);
      		if (t->fd == fd)
			return t->f;
		i = i->next;
	  }	
 return NULL;
}
void exit (int status)
{
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit ();
}
