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
#| | arg 2      | ebp + 12   | There are 9 local variables on the stack.      |
#| --------------            | The first two are temp value that are used to  |
#| --------------            | swap values.                                   |
#| | arg 1      | ebp + 8    | The third one is where the return value of the |
#| --------------            | real syscall function is stored for later use. |
#| --------------            | The rest of the local variables are used to    |
#| | Ret Addr   | ebp + 4    | store the values of the arguments, so they can |
#| --------------            | be passed later to the fake syscall function.  |
#| --------------            |                                                |
#| | EBP        | ebp + 0    | Keep in mind that on x86 all the arguments are |
#| --------------            | passed on the stack, from right to left, which |
#| --------------            | means that the first argument is on the        |
#| | Ret val    | ebp - 4    | hightest position, while the last argument is  |
#| --------------            | on the lowest position.                        |
#| --------------            |                                                |
#| | arg6  copy | ebp - 8    | On the other hand, on x64 arguments are passed |
#| --------------            | via the registers, in this order: RDI, RSI,    |
#| --------------            | RDX, RCX, R8, R9.                              |
#| | arg5  copy | ebp - 12   |                                                |
#| --------------            | In both x86 and x64 the result of the syscall  |
#| --------------            | function is saved in a register. EAX on x86,   |
#| | arg4  copy | ebp - 16   | RAX on x64.                                    |
#| --------------            |                                                |
#| --------------            |                                                |
#| | arg3  copy | ebp - 20   |                                                |
#| --------------            |                                                |
#| --------------            |                                                |
#| | arg2  copy | ebp - 24   |                                                |
#| --------------            |                                                |
#| --------------            |                                                |
#| | arg1  copy | ebp - 28   |                                                |
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
	sub esp, 28 #4 bytes for syscall result and 24 bytes for 6 args

	#Save args on the stack
	mov eax, [ebp + 8]
	mov [ebp - 28], eax
	mov eax, [ebp + 12]
	mov [ebp - 24], eax
	mov eax, [ebp + 16]
	mov [ebp - 20], eax
	mov eax, [ebp + 20]
	mov [ebp - 16], eax
	mov eax, [ebp + 24]
	mov [ebp - 12], eax
	mov eax, [ebp + 28]
	mov [ebp - 8], eax

	#Call real syscall and save return value
	mov eax, 0x10101010 #&iter->rf
	call eax
	mov [ebp - 4], eax

	#Check if procmon's state or the syscall's state are
	#set to 0, and if so, skip calling the fake syscall
	mov eax, [0x10101010] #[&procmon_state]
	cmp eax, 1
	jne skip_ff
	mov eax, [0x10101010] #[&iter->state]
	cmp eax, 1
	jne skip_ff

	#Call fake syscall
	mov eax, 0x10101010 #&iter->ff
	call eax

skip_ff:
	#Restore real syscall result to eax
	mov eax, [ebp - 4]

	#Exit
	mov esp, ebp
	pop ebp
	ret

#elif defined(CONFIG_X86_64)

	#TODO: Do we really need to store RAX? I'm guessing not...
	#TODO: Change a little bit the way this works to make it match with that big comment I wrote for x86!
.global stub
stub:
	#Create stack frame
	push rbp
	mov rbp, rsp
	sub rsp, 56; #8 bytes for syscall result + 48 bytes for 6 args

	#Save args on the stack
	mov [rbp - 16], rdi
	mov [rbp - 24], rsi
	mov [rbp - 32], rdx
	mov [rbp - 40], rcx
	mov [rbp - 48], r8
	mov [rbp - 56], r9

	#Call real syscall and save return value
	mov rax, 0x1010101010101010 #&iter->rf
	mov rdi, [rbp - 16]
	mov rsi, [rbp - 24]
	mov rdx, [rbp - 32]
	mov rcx, [rbp - 40]
	mov r8, [rbp - 48]
	mov r9, [rbp - 56]
	call rax
	mov [rbp - 8], rax

	#Check if procmon's state or the syscall's state are
	#set to 0, and if so, skip calling the fake syscall
	mov eax, [0x1010101010101010] #[&procmon_state]
	cmp eax, 1
	jne skip_ff
	mov eax, [0x1010101010101010] #[&iter->state]
	cmp eax, 1
	jne skip_ff

	#Call fake syscall
	mov rax, 0x1010101010101010 #&iter->ff
	mov rdi, [rbp - 16]
	mov rsi, [rbp - 24]
	mov rdx, [rbp - 32]
	mov rcx, [rbp - 40]
	mov r8, [rbp - 48]
	mov r9, [rbp - 56]
	call rax

skip_ff:
	#Restore real syscall result to rax
	mov rax, [rbp - 8]

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
	sub rsp, 28 #4 bytes for syscall result and 24 bytes for 6 args

	#Save args on the stack
	mov eax, dword [rbp + 8]
	mov dword [rbp - 28], eax
	mov eax, dword [rbp + 12]
	mov dword [rbp - 24], eax
	mov eax, dword [rbp + 16]
	mov dword [rbp - 20], eax
	mov eax, dword [rbp + 20]
	mov dword [rbp - 16], eax
	mov eax, dword [rbp + 24]
	mov dword [rbp - 12], eax
	mov eax, dword [rbp + 28]
	mov dword [rbp - 8], eax

	#Call real syscall and save return value
	mov rax, 0x1010101010101010 #&iter->rf
	call rax
	mov dword [rbp - 4], eax

	#Check if procmon's state or the syscall's state are
	#set to 0, and if so, skip calling the fake syscall
	mov eax, [0x1010101010101010] #[&procmon_state]
	cmp eax, 1
	jne skip_ff32
	mov eax, [0x1010101010101010] #[&iter->state]
	cmp eax, 1
	jne skip_ff32

	#Call fake syscall
	mov rax, 0x1010101010101010 #&iter->ff
	call rax

skip_ff32:
	#Restore real syscall result to rax
	mov eax, dword [rbp - 4]

	#Exit
	mov rsp, rbp
	pop rbp
	ret
#endif

#endif

.att_syntax noprefix