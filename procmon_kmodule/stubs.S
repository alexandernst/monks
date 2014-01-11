.intel_syntax noprefix

#ifdef CONFIG_X86_32

.global stub
stub:
	push ebp
	mov ebp, esp
	sub esp, 32 #4 bytes for tmp, 4 bytes for syscall result and 24 bytes for 6 args

	#Save args on the stack
	mov eax, [ebp + 8]
	mov [ebp - 32], eax
	mov eax, [ebp + 12]
	mov [ebp - 28], eax
	mov eax, [ebp + 16]
	mov [ebp - 24], eax
	mov eax, [ebp + 20]
	mov [ebp - 20], eax
	mov eax, [ebp + 24]
	mov [ebp - 16], eax
	mov eax, [ebp + 28]
	mov [ebp - 12], eax

	#Call atomic_inc (first save first syscall arg)
	mov eax, [ebp - 32]
	mov [ebp - 4], eax
	mov eax, 0x00000000 #&iter->counter
	mov [ebp - 32], eax
	mov eax, 0x00000000 #&atomic_inc
	call eax
	mov eax, [ebp - 4]
	mov [ebp - 32], eax

	#Call real syscall and save return value
	mov eax, 0x00000000 #&iter->rf
	call eax
	mov [ebp - 8], eax

	#TODO Check procmon_state and iter->state for skip

	#Call fake syscall
	mov eax, 0x00000000 #&iter->ff
	call eax

	#Call atomic_dec
	mov eax, 0x00000000 #&iter->counter
	mov [ebp - 32], eax
	mov eax, 0x00000000 #&atomic_dec
	call eax

	#Restore real syscall result to eax
	mov eax, [ebp - 8]

	mov esp, ebp
	pop ebp
	ret

#elif CONFIG_X86_64

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

.global stub_32
stub_32:
	ret

#endif

.att_syntax noprefix