#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

#include <inttypes.h>
#include <threads/vaddr.h>
#include <userprog/pagedir.h>
#include <devices/shutdown.h>
#include <filesys/filesys.h>
#include <filesys/file.h>
#include <userprog/process.h>
#include <string.h>
#include <threads/palloc.h>
#include <threads/malloc.h>
#include <list.h>
#include <devices/input.h>

static void syscall_handler (struct intr_frame *);

void process_close_fds(int fd) { // if 0 then close all
    struct list_elem *e;
    struct thread *cur = thread_current();
    struct list *fdp_list = &cur->fdp_list; // TODO 
    for(e = list_begin(fdp_list); e != list_end(fdp_list); e = list_next(e)) {
        struct fd_process * fdp = list_entry(e, struct fd_process, fd_pr_elem);
        if(fd == fdp->fd || fd == 0) {
            file_close(fdp->f);
//            free(fdp); // BUGGY TODO
            list_remove(e);
        }
    }
}

struct file* get_file_from_fd(int fd) {
    struct list_elem *e;
    struct thread *cur = thread_current();
    struct list *fdp_list = &cur->fdp_list; // TODO 
    for(e = list_begin(fdp_list); e != list_end(fdp_list); e = list_next(e)) {
        struct fd_process * fdp = list_entry(e, struct fd_process, fd_pr_elem);
        if(fd == fdp->fd) {
            return fdp->f;
        }
    }
    return NULL;
}

extern int fd_glob = 1;

int get_next_fd_glob() {
    return ++fd_glob;
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

// static int cnt = 0;

void exit_process(char* process_name, int status) {
    printf ("%s: exit(%d)\n", process_name, status);
    // printf("thread: %s - parent: %s - pp: %s\n\n----\n", thread_current()->name, thread_current()->parent_t->name, thread_current()->parent_t->parent_t->name);
    // printf("%s - %d\n\n-----\n", process_name, thread_current()->tid);
     enum intr_level old_lev;
       old_lev = intr_disable();
    thread_current()->exit_status = status;
    struct thread * cur = thread_current();
    if(thread_current()->parent_t != NULL && cur->parent_t->had_children == true) {
    // printf("%s %s\n\n\n--------\n", thread_current()->tid, process_name);
    struct list * child_plist = &thread_current()->parent_t->child_plist;
    struct list_elem *e;
    if(child_plist != NULL && list_empty(child_plist) == false) {
    tid_t child_tid = thread_current()->tid;
    for(e = list_begin(child_plist); e != list_end(child_plist); e = list_next(e)) {
       struct pchild * pch = list_entry(e, struct pchild, pchild_elem);

       if(pch != NULL && pch->child_tid == child_tid) {
        pch->exit_status = status;
        intr_set_level(old_lev);
        break;
       }
    }
    }
    }
  intr_set_level(old_lev);
    thread_exit(); // free resources and exit process
}

bool is_valid_ptr(char* ptr, uint32_t *pd) {
    if(is_user_vaddr(ptr) == false) return false;
    char* kptr = pagedir_get_page(pd, ptr);
    return (kptr != NULL);
}

// gets next char pointer from ptr, increments by 4 bytes, 
// usage: char *ptr = ...; getnchar(&ptr);
// TODO check invalid ptrs
char* getnchar(char** lp) {
    char* ptr = *lp;
    char* res = (char *) *((int *) ptr);
    struct thread *cur = thread_current();
    // char* kptr = pagedir_get_page(cur->pagedir, res);
    // printf("kptr: %08"PRIx32"\n", kptr);
    // printf("res: %08"PRIx32"\n", res);
    if((is_valid_ptr(res, cur->pagedir)) == false)
        exit_process(cur->name, -1); // TODO is this status correct?
    (*lp) += 4;
    return res;
}

// gets next int occupying 4 bytes from ptr, increments by 4 bytes
int getnint(char** lp) {
    char* ptr = *lp;
    int res = *((int *) ptr);
    struct thread *cur = thread_current();
    if((is_valid_ptr(ptr, cur->pagedir)) == false)
        exit_process(cur->name, -1); // TODO is this status correct?
    (*lp) += 4;
    return res;
}

// gets next unsigned int occupying 4 bytes from ptr, increments by 4 bytes
unsigned getnuint(char** lp) {
    char* ptr = *lp;
    unsigned res = *((unsigned *) ptr);
    struct thread *cur = thread_current();
    if((is_valid_ptr(ptr, cur->pagedir)) == false)
        exit_process(cur->name, -1); // TODO is this status correct?
    (*lp) += 4;
    return res;
}

int cast_unsigned_to_signed(unsigned x) {
    int res = *((int*)(&x));
    return res;
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{   
  // TODO do we need to check this pointer ???
  char *ptr = f->esp;
  struct semaphore file_sema;
  sema_init(&file_sema, 1);

  // TODO is this correct??
  // printf("PTR: %08"PRIx32"\n", ptr); 
  if(is_valid_ptr(ptr, thread_current()->pagedir) == false) {
    exit_process(thread_current()->name, -1);
  }

  // printf("system calll number: %d\n", *((int *)ptr));
  // printf("char* ptr : %08"PRIx32"\n", ptr);
  // hex_dump(0, ptr, 100, true);
  // char* pr = (char*) 0xBFFFFF08;
  // printf("voila : %s\n", pr);
  switch(getnint(&ptr)) {
    case SYS_HALT:
        printf("");
        shutdown_power_off();
        break;
    case SYS_WAIT:
        printf("");
        int pid = getnint(&ptr);
        f->eax = (uint32_t) process_wait(pid);
        break;
    case SYS_OPEN:
        printf("");
        char* file_name = getnchar(&ptr);
        if(strlen(file_name) == 0) 
        {
            f->eax = (uint32_t) -1;
            break;
            //exit_process(thread_current()->name, -1);
        }
        lock_acquire(get_filesys_lock());
        struct thread *cur = thread_current();
        struct file * fil = filesys_open(file_name);
        if(fil == NULL) {
            f->eax = (uint32_t) -1;
            lock_release(get_filesys_lock());
            break;
        }
        
        struct list * fdp_list = &cur->fdp_list;

        struct fd_process * fdpr = (struct fd_process *) malloc(sizeof(struct fd_process)); // TODO free the memory
        if(fdpr == NULL) {
            lock_release(get_filesys_lock());
            exit_process(thread_current()->name, -1);
            break;
        }
        int some = get_next_fd_glob();
        //printf("some: %d\n", some);
        fdpr->fd = some;
        //printf("even before: %d\n", fdpr->fd);
        fdpr->f = fil;

        //printf("middle: %d\n", fdpr->fd);
        list_push_back(fdp_list, &fdpr->fd_pr_elem); // TODO local var fdpr
        //printf("before: %d\n", fdpr->fd);

        f->eax = (uint32_t) fdpr->fd;

        //printf("after: %d\n", (uint32_t) f->eax);

        // if is unnecessary actually TODO
        // if(lock_held_by_current_thread(get_filesys_lock())) 
        lock_release(get_filesys_lock());
        break;
    case SYS_CLOSE:
        printf("");
        int fdc = getnint(&ptr);
        if(fdc < 2) 
            exit_process(thread_current()->name, -1);
        lock_acquire(get_filesys_lock());
        process_close_fds(fdc); 
        lock_release(get_filesys_lock());
        break;
    case SYS_CREATE:
        printf("");
        char* file = getnchar(&ptr);
        unsigned init_size = getnuint(&ptr);
        if(strlen(file) == 0) 
            exit_process(thread_current()->name, -1);
        lock_acquire(get_filesys_lock());
        f->eax = (uint32_t) filesys_create(file, init_size);
        lock_release(get_filesys_lock());
        break;
    case SYS_REMOVE: // added 4 nov
        printf("");
        char* filer = getnchar(&ptr);
        if(strlen(file) == 0) 
            exit_process(thread_current()->name, -1);
        lock_acquire(get_filesys_lock());
        f->eax = (uint32_t) filesys_remove(filer);
        lock_release(get_filesys_lock());
        break;
    case SYS_TELL: // added 4 nov
        printf("");
        int fdt = getnint(&ptr);
        if(fdt == 0 || fdt == 1) { // TODO maybe console telling should be enabled??? 
            exit_process(thread_current()->name, -1); 
            break;
        }
        else {
            lock_acquire(get_filesys_lock()); // REV
            struct file * fil = get_file_from_fd(fdt);
            if(fil == NULL) {
                lock_release(get_filesys_lock()); // REV
                exit_process(thread_current()->name, -1);
                break;
            }
            f->eax = (uint32_t) file_tell(fil);
            lock_release(get_filesys_lock()); // REV
        }
        break;
    case SYS_SEEK: // added 4 nov
        printf("");
        int fds = getnint(&ptr);
        int pos = getnuint(&ptr); // TODO check this argument
        if(fds == 0 || fds == 1) { // TODO maybe console telling should be enabled??? 
            exit_process(thread_current()->name, -1); 
            break;
        }
        else {
            lock_acquire(get_filesys_lock()); // REV
            struct file * fil = get_file_from_fd(fds);
            if(fil == NULL) {
                lock_release(get_filesys_lock()); // REV
                exit_process(thread_current()->name, -1);
                break;
            }
            file_seek(fil, pos); // TODO check ASSERT 
            // TODO do we have to consider cases when pos > file_size???
            lock_release(get_filesys_lock()); // REV
        }
        break;
    case SYS_EXEC:
        printf("");
        char* filee = getnchar(&ptr);
        if(strlen(file) == 0) 
            exit_process(thread_current()->name, -1);

        thread_current()->had_children = true;
        f->eax = (uint32_t) process_execute(filee);
        // thread_block(); // TODO Won't be deadlock???
        struct thread * childt = get_thread_by_tid(f->eax);
        if(childt->loaded_exec == false) { 
            f->eax = -1; // TODO is this correct?
            break;
        }
        break;
    case SYS_FILESIZE:
        printf("");
       int fdf = getnint(&ptr);
       if(fdf < 2) {
        exit_process(thread_current()->name, -1);
        break;
       }
       lock_acquire(get_filesys_lock()); // REV
       struct file *fil_ = get_file_from_fd(fdf);
       if(fil_ == NULL) {
        lock_release(get_filesys_lock());
        exit_process(thread_current()->name, -1);
        break;
       }
       f->eax = (uint32_t) file_length(fil_);
       lock_release(get_filesys_lock());
       break;
    case SYS_READ:
        printf(""); 
        int fdr = getnint(&ptr);
        char* buffer2 = getnchar(&ptr);
        unsigned length2 = getnuint(&ptr);
        
        if(fdr == 0) {
            int b = 0;
            for(b; b < length2; b++) {
               *(buffer2 + b) = (char) input_getc();   // TODO
            }
            f->eax = (uint32_t) length2;
            break;
        }
        else if(fdr == 1) {
            exit_process(thread_current()->name, -1);
            break;
        }   
        else {
            lock_acquire(get_filesys_lock()); // REV
            struct file * fil = get_file_from_fd(fdr);
            if(fil == NULL) {
                lock_release(get_filesys_lock()); // REV
                exit_process(thread_current()->name, -1);
                break;
            }
            int res = file_read(fil, buffer2, length2);
 //           if(res == length2) res = 0;
            f->eax =  (uint32_t) res;
            lock_release(get_filesys_lock()); // REV

        }
        break;
    case SYS_WRITE:
        printf(""); // written as a statement to prevent error with int, etc
        // printf("SYS_WRITE\n");
        int fdw = getnint(&ptr);
        char* buffer = getnchar(&ptr);
        // printf("fd: %d\n", *((int *) ptr));
        // char* buffer = (char *) *((int *)ptr); // correct? o.O
        // hex_dump(0, buffer, 50, true);
        // printf("char* buffer : %08"PRIx32"\n", buffer);
        // printf("char* ptr : %08"PRIx32"\n", ptr);
        // unsigned length = *((int*)ptr);
        unsigned length = getnuint(&ptr);
        // printf("fd: %d\n", fd);
        // printf("length: %d\n", length);
        // printf("buffer :%s\n", buffer);
        if(fdw == 0) {
            exit_process(thread_current()->name, -1); 
            break;
        }
        else if(fdw == 1) {
            // TODO if there are two many bytes!
            putbuf(buffer, length);
            f->eax = (uint32_t) length;
            break;
        }   
        else {
           lock_acquire(get_filesys_lock()); // REV
            struct file * fil = get_file_from_fd(fdw);
            if(fil == NULL) {
                lock_release(get_filesys_lock()); // REV
                exit_process(thread_current()->name, -1);
                break;
            }
            f->eax = (uint32_t) file_write(fil, buffer, length);
            lock_release(get_filesys_lock()); // REV
        }
        break;
    case SYS_EXIT:
        printf("");
        int status = getnint(&ptr);
        exit_process(thread_current()->name, status); 
        break;
    default: 
        printf ("system call!\n");
        thread_exit();
  }
}
