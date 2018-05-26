#include <vm/page.h>
#include <threads/vaddr.h>
#include "filesys/filesys.h"
#include "filesys/file.h"

static unsigned page_hash (const struct hash_elem *p_, void *aux UNUSED)
{
    const struct vm_spt_e *p = hash_entry (p_, struct vm_spt_e, elem);
    return hash_int ((int)p->vaddr); // we hash by the virtual address
}

static bool page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED)
{
    const struct vm_spt_e *a = hash_entry (a_, struct vm_spt_e, elem);
    const struct vm_spt_e *b = hash_entry (b_, struct vm_spt_e, elem);
    return a->vaddr < b->vaddr;
}

void vm_spt_init(struct hash* spt) {
    hash_init(spt, page_hash, page_less, NULL);
}

// Clears all the allocated data from vm_spt_e
void deallocate_vm_spt_e(struct hash_elem *e, void* aux UNUSED) {
    struct vm_spt_e* a = hash_entry (e, struct vm_spt_e, elem);
    if(a->file != NULL) {
     //   file_close(a->file); // TODO: CLOSE THE FILE
    }
    free(a); // TODO: is this correct???
}

void vm_spt_destroy(struct hash* spt) {
    hash_destroy(spt, deallocate_vm_spt_e); // do we have to free vm_spt_e objects ? TODO
}

void vm_spt_add_file_entry(struct hash* spt, struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable) {
    struct vm_spt_e* a = malloc(sizeof(struct vm_spt_e));
    if(a == NULL) {
        PANIC("NO MEMORY FOR VM_SPT_E CREATION");
    }
    a->file = file;
    a->ofs = ofs;
    a->vaddr = pg_round_down(upage);
    a->read_bytes = read_bytes;
    a->zero_bytes = zero_bytes;
    a->writable = writable;
    a->is_swap = a->is_page = false;
    a->is_file = true;
    hash_insert(spt, &a->elem);
}

struct vm_spt_e* vm_spt_find_e(struct hash* spt, void* addr) {
    if(spt == NULL) {
        PANIC("SPT IS NULL!");
    }
     struct vm_spt_e p;
     struct hash_elem *e;

     p.vaddr = pg_round_down(addr);
     e = hash_find (spt, &p.elem);
     
     return e != NULL ? hash_entry (e, struct vm_spt_e, elem) : NULL;
}



