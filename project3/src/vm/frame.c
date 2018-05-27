#include "vm/frame.h"

static void add_to_frame_table(void *physe_page) {
	lock_acquire(&frame_lock);
	
	struct frame_table_e *elem = malloc(sizeof(struct frame_table_e));
	elem->physe_page = physe_page;
	list_push_back(&frame_table, &elem->elem);
	
	lock_release(&frame_lock);
}

void frame_table_init(void) {
	 list_init(&frame_table);
	 lock_init(&frame_lock);
}

void* frame_table_alloc(enum palloc_flags f){
	if(!f || !PAL_USER) return NULL;
	
	void *fra = NULL;
	fra = palloc_get_page(f);
	if(fra) add_to_frame_table(fra); 
	else PANIC("NO FRAMES");
	return fra;
	
}
void frame_table_free(void *physe_page){
	lock_acquire(&frame_lock);
	struct list_elem *i = list_begin(&frame_table);
	while(i  != list_end(&frame_table) ) {
		struct frame_table_e *temp = list_entry(i, struct frame_table_e, elem);
		if(temp->physe_page == physe_page) {
			list_remove(i);
			free(temp);
			palloc_free_page(physe_page);
			break;
		}
		i=list_next(i);
	}
	lock_release(&frame_lock);
}
