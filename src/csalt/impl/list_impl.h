#ifndef CSALT_IMPL_LIST_IMPL_H
#define CSALT_IMPL_LIST_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif

struct resource_heap_data {
	struct csalt_heap heap;
	struct csalt_store_list *list;
	size_t begin;
	size_t end;
	csalt_store_block_fn *block;
	void *data_param;
	int error;
	csalt_store_list_receive_split_fn *receive_split_list;
};

int manage_heap_data(struct resource_heap_data *heap_data);

#ifdef __cplusplus
} // extern "C"
#endif 

#endif //CSALT_IMPL_LIST_IMPL_H
