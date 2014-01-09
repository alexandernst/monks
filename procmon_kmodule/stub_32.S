.intel_syntax noprefix
.global stub_32
stub_32:
	push ebp;
	mov ebp, esp;
	sub esp, 28; #4 bytes for syscall result + 24 bytes for 6 args

	#Save args on the stack
	mov eax, [ebp + 8]; 
	mov [ebp - 28], eax;
	mov eax, [ebp + 12];
	mov [ebp - 24], eax;
	mov eax, [ebp + 16];
	mov [ebp - 20], eax;
	mov eax, [ebp + 20];
	mov [ebp - 16], eax;
	mov eax, [ebp + 24];
	mov [ebp - 12], eax;
	mov eax, [ebp + 28];
	mov [ebp - 8], eax; 

	mov eax, 0x00000000; #&iter->rf;
	call eax;

	mov esp, ebp;
	pop ebp;
	ret;
.att_syntax noprefix