#include "vm/swap.h"
#include "threads/vaddr.h"
#include "debug.h"
#include "stdint.h"
#include "stdbool.h"

void vm_swap_init() {
    // Initialize the lock, mutex
    lock_init(&vm_swap_lock);
    
    // Initialize the swap slots
    vm_swap_block = block_get_role(BLOCK_SWAP);
    if(vm_swap_block == NULL) {
        PANIC("Swap block is not found or not initialized!");
    }

    // Initialize the bitmap
    vm_swap_bm = bitmap_create(block_size(vm_swap_block) / slots_in_page());
    if(vm_swap_bm == NULL) {
        PANIC("Bitmap for swap slots have failed to be allocated!");
    }
}

void vm_swap_destroy() {
    ASSERT(vm_swap_block && vm_swap_bm);
    bitmap_destroy(vm_swap_bm);
    // TODO destroy swap..
}

unsigned int vm_swap_in(void* frame) {
    ASSERT(vm_swap_block && vm_swap_bm);
    lock_acquire(&vm_swap_lock);
    frame = pg_round_down(frame); // TODO is this correct ?
    unsigned int index = bitmap_scan_and_flip(vm_swap_bm, 0, 1, 0);
    if(index == BITMAP_ERROR) {
        PANIC("No free swap slot left!");
    }
    int i;
    for(i = 0; i < slots_in_page(); i++) {
        char* frame_ch = (char *) frame;
        block_write(vm_swap_block, i + index * slots_in_page(), frame_ch + i * BLOCK_SECTOR_SIZE);
    }
    lock_release(&vm_swap_lock);
    return index;
}

void vm_swap_out(void* frame, unsigned int index) {
   ASSERT(vm_swap_block && vm_swap_bm);
   lock_acquire(&vm_swap_lock);
   frame = pg_round_down(frame); // TODO is this correct ?
   bool is_located = bitmap_test(vm_swap_bm, index);
   if(is_located == false) {
        PANIC("The specified index is not located in the swap slot!");
   } 
   bitmap_reset(vm_swap_bm, index);
   int i;
   for(i = 0; i < slots_in_page(); i++) {
        char* frame_ch = (char *) frame;
        block_read(vm_swap_block, i + index * slots_in_page(), frame_ch + i * BLOCK_SECTOR_SIZE);
   }
   lock_release(&vm_swap_lock);
}

// TODO: now 1024 bytes in a page, and 512 in block's slot, so 2 slots in page
int slots_in_page() {
    return PGSIZE / BLOCK_SECTOR_SIZE;
}
