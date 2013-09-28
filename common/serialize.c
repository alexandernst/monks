#include "serialize.h"

char *serialize_membuffer(membuffer *buffer){
	char *data = kmalloc(sizeof(size_t) + buffer->len, GFP_KERNEL);

	memcpy(data, &buffer->len, sizeof(size_t));
	memcpy(data + sizeof(size_t), buffer->data, buffer->len);

	return data;
}

membuffer *serialize_syscall_info(syscall_info *i){
	membuffer *buffer = kmalloc(sizeof(membuffer), GFP_KERNEL);
	buffer->data = NULL;
	buffer->len = 0;

	add_chunk(buffer, (void *)i->pname, strlen(i->pname) + 1);
	add_chunk(buffer, (void *)&i->pid, sizeof(pid_t));
	add_chunk(buffer, (void *)i->operation, strlen(i->operation) + 1);
	add_chunk(buffer, (void *)i->path, strlen(i->path) + 1);
	add_chunk(buffer, (void *)i->result, strlen(i->result) + 1);
	add_chunk(buffer, (void *)i->details, strlen(i->details) + 1);

	return buffer;
}

void add_chunk(membuffer *buffer, void *chunk, size_t size){
	if(buffer->data == NULL){
		buffer->data = kmalloc(sizeof(size_t) + size, GFP_KERNEL);
		buffer->len = sizeof(size_t) + size;

		memcpy(buffer->data, &size, sizeof(size_t));
		memcpy(buffer->data + sizeof(size_t), chunk, size);
	}else{
		buffer->data = krealloc(buffer->data, buffer->len + sizeof(size_t) + size, GFP_KERNEL);

		memcpy(buffer->data + buffer->len, &size, sizeof(size_t));
		memcpy(buffer->data + buffer->len + sizeof(size_t), chunk, size);

		buffer->len += sizeof(size_t) + size;
	}
}