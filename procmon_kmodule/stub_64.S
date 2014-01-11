.global stub_32
stub_32:
	ret

.intel_syntax noprefix
.global stub
stub:
	#TODO: Do we really need to store RAX? I'm guessing not...
	#TODO: Change a little bit the way this works to make it match with that big comment I wrote for x86!
	push rbp
	mov rbp, rsp
	sub rsp, 64; #8 bytes for rax content + 48 bytes for 6 args + 8 bytes for syscall resul

	mov [rbp - 8], rax
	mov [rbp - 16], rdi
	mov [rbp - 24], rsi
	mov [rbp - 32], rdx
	mov [rbp - 40], rcx
	mov [rbp - 48], r8
	mov [rbp - 56], r9

	mov rax, 0x1010101010101010 #&atomic_inc
	mov rdi, 0x1010101010101010 #&iter->counter
	call rax

	mov rax, 0x1010101010101010 #&iter->rf
	mov rdi, [rbp - 16]
	mov rsi, [rbp - 24]
	mov rdx, [rbp - 32]
	mov rcx, [rbp - 40]
	mov r8, [rbp - 48]
	mov r9, [rbp - 56]
	call rax

	mov [rbp - 64], rax

	mov eax, [0x1010101010101010] #[&procmon_state]
	cmp eax, 1
	jne skip_ff

	mov eax, [0x1010101010101010] #[&iter->state]
	cmp eax, 1
	jne skip_ff

	mov rax, 0x1010101010101010 #&iter->ff
	mov rdi, [rbp - 16]
	mov rsi, [rbp - 24]
	mov rdx, [rbp - 32]
	mov rcx, [rbp - 40]
	mov r8, [rbp - 48]
	mov r9, [rbp - 56]
	call rax

skip_ff:
	mov rax, 0x1010101010101010 #&atomic_dec
	mov rdi, 0x1010101010101010 #&iter->counter
	call rax

	mov rax, [rbp - 64]

	mov rsp, rbp
	pop rbp
	ret
.att_syntax noprefix