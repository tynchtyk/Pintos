#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include <threads/synch.h>
#include <filesys/file.h>

struct pchild {
    int exit_status;
    tid_t child_tid; 
    struct list_elem pchild_elem;
};

struct wchild {
    tid_t child_tid; 
    struct list_elem wchild_elem;
};

static struct lock filesys_lock;
struct lock* get_filesys_lock(void);

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

bool load_file (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t page_read_bytes, uint32_t page_zero_bytes, bool writable);

#endif /* userprog/process.h */
