#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <string.h>
#include <stdbool.h>
#include "filesys/file.h"
#include "threads/interrupt.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "userprog/syscall.h"
#include <hash.h>
#include "vm/frame.h"

#include "filesys/off_t.h"

struct supplement_page_table_e{
  struct file *file;
  void *user_vaddr;
  bool writable;

  struct hash_elem elem;
  
  size_t offset;
  off_t read_bytes;
  off_t zero_bytes;
  
};

//start finish
void supplement_page_table_init (struct hash *Hash);
void supplement_page_table_destroy (struct hash *Hash);

//editing table
struct supplement_page_table_e* construct_table_element (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable);

bool add_file_to_supplement_page_table (struct file *file, int32_t offset, uint8_t *user_page,
			     uint32_t read_bytes, uint32_t zero_bytes, bool writable);
struct supplement_page_table_e* get_elem_from_table(void *user_vaddr);

//hash helper functions
unsigned hash_func(const struct hash_elem *elem, void *aux UNUSED);
bool less_func(const struct hash_elem *elem1, const struct hash_elem *elem2, void *aux UNUSED);
void destroy_func(struct hash_elem *elem, void *aux UNUSED);


////

bool load_file (struct file *file, off_t ofs, uint8_t *upage,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable);

#endif /* vm/page.h */