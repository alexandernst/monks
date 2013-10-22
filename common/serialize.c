#include "serialize.h"

membuffer *serialize_syscall_info(syscall_info *i){
	membuffer *buffer = new(sizeof(membuffer));
	buffer->data = NULL;
	buffer->len = 0;

	if(!add_chunk(buffer, (void *)i->pname, strlen(i->pname) + 1)){
		return NULL;
	}
	if(!add_chunk(buffer, (void *)&i->pid, sizeof(pid_t))){
		return NULL;
	}
	if(!add_chunk(buffer, (void *)i->operation, strlen(i->operation) + 1)){
		return NULL;
	}
	if(!add_chunk(buffer, (void *)i->path, strlen(i->path) + 1)){
		return NULL;
	}
	if(!add_chunk(buffer, (void *)i->result, strlen(i->result) + 1)){
		return NULL;
	}
	if(!add_chunk(buffer, (void *)i->details, strlen(i->details) + 1)){
		return NULL;
	}

	return buffer;
}

int add_chunk(membuffer *buffer, void *chunk, size_t size){
	byte *tmp;

	if(buffer->data == NULL){	
		tmp = new(sizeof(size_t) + size);
		if(!tmp){
			return 0;
		}else{
			buffer->data = tmp;
		}
		buffer->len = sizeof(size_t) + size;

		memcpy(buffer->data, &size, sizeof(size_t));
		memcpy(buffer->data + sizeof(size_t), chunk, size);
	}else{
		tmp = renew(buffer->data, buffer->len + sizeof(size_t) + size);
		if(!tmp){
			del(buffer->data);
			del(buffer);
			return 0;
		}else{
			buffer->data = tmp;
		}

		memcpy(buffer->data + buffer->len, &size, sizeof(size_t));
		memcpy(buffer->data + buffer->len + sizeof(size_t), chunk, size);

		buffer->len += sizeof(size_t) + size;
	}

	return 1;
}