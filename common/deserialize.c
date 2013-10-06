#include "deserialize.h"

membuffer *deserialize_membuffer(char *data){
	size_t size;
	membuffer *buffer = new(sizeof(membuffer));

	if(!buffer){
		return NULL;
	}

	memcpy(&size, data, sizeof(size_t));

	buffer->len = size;
	buffer->data = new(size);

	if(!buffer->data){
		del(buffer);
		return NULL;
	}

	memcpy(buffer->data, data + sizeof(size_t), size);

	return buffer;
}

syscall_info *deserialize_syscall_info(membuffer *buffer){
	syscall_info *i = new(sizeof(syscall_info));
	if(!i){
		return NULL;
	}

	i->pname = (char *)get_chunk(buffer);
	i->pid = *(pid_t *)get_chunk(buffer);
	i->operation = (char *)get_chunk(buffer);
	i->path = (char *)get_chunk(buffer);
	i->result = (char *)get_chunk(buffer);
	i->details = (char *)get_chunk(buffer);

	return i;
}

void *get_chunk(membuffer *buffer){
	void *chk;
	size_t size;

	size = *(size_t*)buffer->data;
	buffer->data += sizeof(size_t);
	chk = buffer->data;
	buffer->data += size;
	buffer->len -= size + sizeof(size_t);
	return chk;
}