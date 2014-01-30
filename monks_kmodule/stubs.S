#/****************************************************************************\
#|                           |                                                |
#| --------------            | This is how the stack looks like when the stub |
#| | arg 6      | ebp + 28   | is executed and the args are saved.            |
#| --------------            |                                                |
#| --------------            | Note that this is valid for both x86 and x64   |
#| | arg 5      | ebp + 24   | architectures. The only differences are the    |
#| --------------            | size of each value, which on x86 is 4 bytes,   |
#| --------------            | and on x64 is 8 bytes, and the register used   |
#| | arg 4      | ebp + 20   | to access the stack, which on x86 is called    |
#| --------------            | EBP, while on x64 is called RBP.               |
#| --------------            |                                                |
#| | arg 3      | ebp + 16   | This means that each value after EBP should be |
#| --------------            | multiplied by 2 for x64.                       |
#| --------------            |                                                |
#| | arg 2      | ebp + 12   | There are 7 local variables on the stack.      |
#| --------------            | The first one is where the return value of the |
#| --------------            | real syscall function is stored for later use. |
#| | arg 1      | ebp + 8    | The rest of the local variables are used to    |
#| --------------            | store the values of the arguments, so they can |
#| --------------            | be passed later to the fake syscall function.  |
#| | Ret Addr   | ebp + 4    |                                                |
#| --------------            | Keep in mind that on x86 all the arguments are |
#| --------------            | passed on the stack, from right to left, which |
#| | EBP        | ebp + 0    | means that the first argument is on the        |
#| --------------            | hightest position, while the last argument is  |
#| --------------            |
#| | Hook info  | ebp - 4    |
#| --------------            |
#| --------------            | on the lowest position.                        |
#| | Ret val    | ebp - 8    |                                                |
#| --------------            | On the other hand, on x64 arguments are passed |
#| --------------            | via the registers, in this order: RDI, RSI,    |
#| | arg6  copy | ebp - 12   | RDX, RCX, R8, R9.                              |
#| --------------            |                                                |
#| --------------            | In both x86 and x64 the result of the syscall  |
#| | arg5  copy | ebp - 16   | function is saved in a register. EAX on x86,   |
#| --------------            | RAX on x64.                                    |
#| --------------            |                                                |
#| | arg4  copy | ebp - 20   |                                                |
#| --------------            |                                                |
#| --------------            |                                                |
#| | arg3  copy | ebp - 24   |                                                |
#| --------------            |                                                |
#| --------------            |                                                |
#| | arg2  copy | ebp - 28   |                                                |
#| --------------            |                                                |
#| --------------            |                                                |
#| | arg1  copy | ebp - 32   |                                                |
#| --------------            |                                                |
#|                           |                                                |
#\****************************************************************************/

.intel_syntax noprefix

#ifdef CONFIG_X86_32

.global stub
stub:
	#Create stack frame
	push ebp
	mov ebp, esp
	sub esp, 32 #4 bytes for hook info, 4 bytes for result, 24 bytes for 6 args

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

	#Check if monks's state or the syscall's state are
	#set to 0, and if so, skip calling the pre-hook
	mov eax, [0x10101010] #[&monks_state]
	cmp eax, 1
	jne skip_pre
	mov eax, [0x10101010] #[&iter->state]
	cmp eax, 1
	jne skip_pre

	#Call pre-hook
	mov eax, 0x10101010 #&iter->pre
	call eax

skip_pre:
	#Call real syscall and save return value
	mov eax, 0x10101010 #&iter->rf
	call eax
	mov [ebp - 8], eax

	#Check if monks's state or the syscall's state are
	#set to 0, and if so, skip calling the post-hook
	mov eax, [0x10101010] #[&monks_state]
	cmp eax, 1
	jne skip_post
	mov eax, [0x10101010] #[&iter->state]
	cmp eax, 1
	jne skip_post

	#Call post-hook
	mov eax, 0x10101010 #&iter->post
	call eax

skip_post:
	#Restore real syscall result to eax
	mov eax, [ebp - 8]

	#Exit
	mov esp, ebp
	pop ebp
	ret

#elif defined(CONFIG_X86_64)

.global stub
stub:
	#Create stack frame
	push rbp
	mov rbp, rsp
	sub rsp, 64 #8 bytes for hook info, 8 bytes for result, 48 bytes for 6 args

	#Save args on the stack
	mov [rbp - 24], rdi
	mov [rbp - 32], rsi
	mov [rbp - 40], rdx
	mov [rbp - 48], rcx
	mov [rbp - 56], r8
	mov [rbp - 64], r9

	#Check if monks's state or the syscall's state are
	#set to 0, and if so, skip calling the pre-hook
	mov eax, [0x1010101010101010] #[&monks_state]
	cmp eax, 1
	jne skip_pre
	mov eax, [0x1010101010101010] #[&iter->state]
	cmp eax, 1
	jne skip_pre

	#Call pre-hook
	mov rax, 0x1010101010101010 #&iter->pre
	mov rdi, [rbp - 24]
	mov rsi, [rbp - 32]
	mov rdx, [rbp - 40]
	mov rcx, [rbp - 48]
	mov r8, [rbp - 56]
	mov r9, [rbp - 64]
	call rax

skip_pre:
	#Call real syscall and save return value
	mov rax, 0x1010101010101010 #&iter->rf
	mov rdi, [rbp - 24]
	mov rsi, [rbp - 32]
	mov rdx, [rbp - 40]
	mov rcx, [rbp - 48]
	mov r8, [rbp - 56]
	mov r9, [rbp - 64]
	call rax
	mov [rbp - 16], rax

	#Check if monks's state or the syscall's state are
	#set to 0, and if so, skip calling the post-hook
	mov eax, [0x1010101010101010] #[&monks_state]
	cmp eax, 1
	jne skip_post
	mov eax, [0x1010101010101010] #[&iter->state]
	cmp eax, 1
	jne skip_post

	#Call post-hook
	mov rax, 0x1010101010101010 #&iter->post
	mov rdi, [rbp - 24]
	mov rsi, [rbp - 32]
	mov rdx, [rbp - 40]
	mov rcx, [rbp - 48]
	mov r8, [rbp - 56]
	mov r9, [rbp - 64]
	call rax

skip_post:
	#Restore real syscall result to rax
	mov rax, [rbp - 16]

	#Exit
	mov rsp, rbp
	pop rbp
	ret

#if defined(CONFIG_IA32_EMULATION)

.global stub_32
stub_32:
	#Create stack frame
	push rbp
	mov rbp, rsp
	sub rsp, 36 #8 bytes for hook info, 4 bytes for result, 24 bytes for 6 args

	#Save args on the stack
	mov eax, [rbp + 8]
	mov [rbp - 36], eax
	mov eax, [rbp + 12]
	mov [rbp - 32], eax
	mov eax, [rbp + 16]
	mov [rbp - 28], eax
	mov eax, [rbp + 20]
	mov [rbp - 24], eax
	mov eax, [rbp + 24]
	mov [rbp - 20], eax
	mov eax, [rbp + 28]
	mov [rbp - 16], eax

	#Check if monks's state or the syscall's state are
	#set to 0, and if so, skip calling the pre-hook
	mov eax, [0x1010101010101010] #[&monks_state]
	cmp eax, 1
	jne skip_pre32
	mov eax, [0x1010101010101010] #[&iter->state]
	cmp eax, 1
	jne skip_pre32

	#Call pre-hook
	mov rax, 0x1010101010101010 #&iter->pre
	call rax

skip_pre32:
	#Call real syscall and save return value
	mov rax, 0x1010101010101010 #&iter->rf
	call rax
	mov [rbp - 12], eax

	#Check if monks's state or the syscall's state are
	#set to 0, and if so, skip calling the post-hook
	mov eax, [0x1010101010101010] #[&monks_state]
	cmp eax, 1
	jne skip_post32
	mov eax, [0x1010101010101010] #[&iter->state]
	cmp eax, 1
	jne skip_post32

	#Call post-hook
	mov rax, 0x1010101010101010 #&iter->post
	call rax

skip_post32:
	#Restore real syscall result to rax
	mov eax, [rbp - 12]

	#Exit
	mov rsp, rbp
	pop rbp
	ret
#endif

#endif

.att_syntax noprefix