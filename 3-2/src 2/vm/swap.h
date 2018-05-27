#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "threads/synch.h"
#include "devices/block.h"
#include "lib/kernel/bitmap.h"

struct bitmap* vm_swap_bm;
struct block* vm_swap_block;
struct lock vm_swap_lock;

void vm_swap_init(void);
void vm_swap_destroy(void);
unsigned int vm_swap_in(void* frame); // put the given frame to the block
void vm_swap_out(void* frame, unsigned int index); // swap from the swap slot to the frame

#endif
