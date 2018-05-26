#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/palloc.h"
#include <stdbool.h>
#include <stdint.h>
#include "lib/kernel/list.h"

struct list vm_frame_t;

struct vm_frame_e {
    void* vm_frame; 
    struct list_elem elem; 
};

void vm_frame_init(void);
void* vm_frame_alloc(enum palloc_flags flags);
void vm_frame_free(void *);

#endif 
