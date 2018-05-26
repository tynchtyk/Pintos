#include "vm/frame.h"
#include "threads/malloc.h"
#include "threads/synch.h"

static struct lock vm_frame_lock;

void vm_frame_init() {
    lock_init(&vm_frame_lock);
    list_init(&vm_frame_t);
}

void* vm_frame_alloc(enum palloc_flags flags) {
    void* frame = NULL;
    if(flags & PAL_USER) {
        if(flags & PAL_ZERO) {
            frame = palloc_get_page(PAL_USER | PAL_ZERO);    
        }
        else {
            frame = palloc_get_page(PAL_USER);
        }
    } else {
        return NULL; // Error: allocating from kernel pool
    }
    if(frame == NULL) {
        PANIC("No free frames!"); // TODO: do swapping and frame replacement
    } else {
        // Add the frame to the frame table
        struct vm_frame_e *entry;
        entry = malloc(sizeof(struct vm_frame_e));
        entry->vm_frame = frame;
        lock_acquire(&vm_frame_lock);
        list_push_back(&vm_frame_t, &entry->elem);
        lock_release(&vm_frame_lock);
        return frame;
    }
}

void vm_frame_free(void* frame) {
    lock_acquire(&vm_frame_lock);
    struct list_elem *e;
    for(e = list_begin(&vm_frame_t); e != list_end(&vm_frame_t); e = list_next(e)) {
        struct vm_frame_e* entry = list_entry(e, struct vm_frame_e, elem);
        if(entry == frame) {
            list_remove(e);
            palloc_free_page(entry->vm_frame);
            free(entry);
            break;
        }
    }
    lock_release(&vm_frame_lock);
}
