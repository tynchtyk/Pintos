#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/malloc.h"
#include "userprog/pagedir.h"
#include "list.h"
#include <stdbool.h>
#include <stdint.h>

struct lock frame_lock;
struct list frame_table;

struct frame_table_e {
    void* physe_page; // Each entry in the frame table contains a pointer to the page 
    struct list_elem elem;
};

void frame_table_init(void);

void* frame_table_alloc(enum palloc_flags f);
void frame_table_free(void* physe_page);

#endif 