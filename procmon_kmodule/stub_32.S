.intel_syntax noprefix
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
.att_syntax noprefix