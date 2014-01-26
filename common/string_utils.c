#include "string_utils.h"

char *format(char *fmt, types type, ...){
	int err;
	char *tmp;
	va_list ap;
	int max_len = 2048;

	tmp = new(max_len);
	if(!tmp){
		return NULL;
	}

	va_start(ap, type);

	if(type == NUMBER){
		int val = va_arg(ap, int);

		err = snprintf(tmp, max_len, fmt, val);
	}else if(type == STRING){
		char *val = va_arg(ap, char*);

		err = snprintf(tmp, max_len, fmt, val);
	}

	va_end(ap);

	if(err < 0){
		del(tmp);
		return NULL;
	}

	return tmp;
}

char *get_str_info(syscall_intercept_info *i){
	int err;
	int max_len = 2048;
	char *tmp = new(max_len);
	if(!tmp){
		return NULL;
	}

	err = snprintf(tmp,
		max_len,
		"%-15.20s"       /* Process name */
		"%10u"           /* PID          */
		" %-15.10s"      /* Operation    */
		"%-50.45s"       /* Path         */
		"%-10.10s"       /* Result       */
		" %-200.200s\n", /* Details      */

		i->pname,
		i->pid,
		i->operation,
		i->path,
		i->result,
		i->details
	);

	if(err < 0){
		del(tmp);
		return NULL;
	}

	rstrip(tmp);
	return tmp;
}

void rstrip(char *string){
	int l;
	if(!string){
		return;
	}
	l = strlen(string) - 1;
	while(isspace(string[l]) && l >= 0){
		string[l--] = 0;
	}
}

void lstrip(char *string){
	int i, l;
	if(!string){
		return;
	}
	l = strlen(string);
	while(isspace(string[(i = 0)])){
		while(i++ < l){
			string[i-1] = string[i];
		}
	}
}