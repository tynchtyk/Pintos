#include "vm/page.h"

void supplement_page_table_init (struct hash *Hash){
	   ASSERT(Hash != NULL);
	 hash_init (Hash,  hash_func, less_func, NULL);
}

void supplement_page_table_destroy (struct hash *Hash){
	   ASSERT(Hash != NULL);
	hash_destroy (Hash,  destroy_func);
}
struct supplement_page_table_e* construct_table_element (struct file *file, off_t offset, uint8_t *user_page,
			     uint32_t read_bytes, uint32_t zero_bytes,  bool writable){
	struct supplement_page_table_e *spte = malloc(sizeof(struct supplement_page_table_e));
	if (spte) {
  		spte->zero_bytes = zero_bytes;
	  	spte->read_bytes = read_bytes;
		spte->file = file;
  		spte->user_vaddr = user_page;
	 	spte->offset = offset;
	  	spte->writable = writable;
		return spte;
	}
	return NULL;
}

bool add_file_to_supplement_page_table (struct file *file, off_t offset, uint8_t *user_page,
			     uint32_t read_bytes, uint32_t zero_bytes,  bool writable){
	struct supplement_page_table_e *spte = construct_table_element(file, offset, user_page, read_bytes, zero_bytes, writable);
	if (spte) {
		hash_insert(&thread_current()->spt, &spte->elem);
		return true;
	}
	else false;
}


struct supplement_page_table_e* get_elem_from_table( void *vaddr){
		struct supplement_page_table_e spte;
		spte.user_vaddr = pg_round_down(vaddr);
		struct hash* virtual_memory = &thread_current()->spt;
		struct hash_elem *FIFALA = hash_find(virtual_memory, &spte.elem);
		struct supplement_page_table_e* answer = NULL;
		if(FIFALA) 
			answer = hash_entry(FIFALA,  struct supplement_page_table_e, elem);
		return answer;
}



//hash helper functions
unsigned hash_func(const struct hash_elem *elem, void *aux UNUSED){
	return hash_int(hash_entry(elem, struct supplement_page_table_e, elem)->user_vaddr );
}

bool less_func(const struct hash_elem *elem1, const struct hash_elem *elem2, void *aux UNUSED)
{
	bool answer = (hash_entry(elem1, struct supplement_page_table_e, elem)->user_vaddr 
			< 
			hash_entry(elem2, struct supplement_page_table_e, elem)->user_vaddr) ? true: false ;
	return answer;
}
void destroy_func(struct hash_elem *elem, void *aux UNUSED) {
	struct supplement_page_table_e *spte = hash_entry(elem, struct supplement_page_table_e, elem);
//	free_page_vaddr(spte->vaddr);
	//here we need to do something
	
	free(spte);
}
// handler
bool load_file (struct file *file, off_t offset, uint8_t *user_page,
              uint32_t read_bytes, uint32_t zero_bytes, bool writable){
	
	 file_seek (file, offset);
         uint8_t *kpage = frame_table_alloc (PAL_USER);
	 if (kpage == NULL)
	         return false;

	 if (file_read (file, kpage, read_bytes) != (int) read_bytes)       {
	          frame_table_free (kpage);
	          return false; 
	 }
	memset (kpage + read_bytes, 0, zero_bytes);

	if (!install_page (user_page, kpage, writable))         {
	          frame_table_free (kpage);
	          return false; 
	}
	return true;

}

