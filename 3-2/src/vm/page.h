#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stdbool.h>
#include <stdint.h>
#include "lib/kernel/hash.h"
#include "filesys/file.h"
#include <debug.h>
#include <stddef.h>
#include <random.h>
#include <stdio.h>
#include <string.h>

struct vm_spt_e { // virtual memory supplementary page table entry
    void* vaddr; // virtual address
    bool writable;
    
    struct hash_elem elem;
    
    // for files
    struct file* file;
    bool is_file;
    off_t ofs;
    uint32_t read_bytes;
    uint32_t zero_bytes;
    
    bool is_swap;
    bool is_page;
};

void vm_spt_init(struct hash* spt);
void vm_spt_destroy(struct hash* spt);
void vm_spt_add_file_entry(struct hash* spt, struct file *file, off_t ofs, uint8_t *upage,
                           uint32_t read_bytes, uint32_t zero_bytes, bool writable);

struct vm_spt_e* vm_spt_find_e(struct hash* spt, void* addr);


#endif
