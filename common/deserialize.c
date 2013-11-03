#include "deserialize.h"

syscall_info *deserialize_syscall_info(membuffer *buffer){
	void *tmp;
	syscall_info *i = new(sizeof(syscall_info));
	
	if(!i){
		return NULL;
	}

	i->pname = (char *)get_chunk(buffer);

	tmp = get_chunk(buffer);
	i->pid = *(pid_t *)tmp;
	del(tmp);

	i->operation = (char *)get_chunk(buffer);
	i->path = (char *)get_chunk(buffer);
	i->result = (char *)get_chunk(buffer);
	i->details = (char *)get_chunk(buffer);

	return i;
}

void *get_chunk(membuffer *buffer){
	size_t size;
	void *chunk;
	membuffer *tmp;

	memcpy(&size, buffer->data, sizeof(size_t));
	chunk = new(size);
	memcpy(chunk, buffer->data + sizeof(size_t), size);

	tmp = new(sizeof(membuffer));
	tmp->len = buffer->len - sizeof(size_t) - size;
	tmp->data = new(tmp->len);

	memcpy(tmp->data, buffer->data + sizeof(size_t) + size, tmp->len);

	del(buffer->data);
	buffer->data = tmp->data;
	buffer->len = tmp->len;
	del(tmp);

	return chunk;
}