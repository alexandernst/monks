#include "udis_utils.h"

uint64_t ud_get_stub_size(void *entry){
	ud_t ud;

	ud_init(&ud);
	ud_set_mode(&ud, BITS_PER_LONG);
	ud_set_vendor(&ud, UD_VENDOR_ANY);
	ud_set_input_buffer(&ud, entry, 512);

	while(ud_disassemble(&ud)){
		if(ud.mnemonic == UD_Iret){
			return ud_insn_off(&ud) + ud_insn_len(&ud);
		}
	}

	return 0;
}

void ud_patch_addr(void *entry, void *addr){
	ud_t ud;
	char *match;
	void *patch_addr;
	int offset, ptr_size;

	ptr_size = sizeof(void *);

#ifdef CONFIG_X86_32
	match = "\x10\x10\x10\x10";
#elif defined(CONFIG_X86_64)
	match = "\x10\x10\x10\x10\x10\x10\x10\x10";
#endif

	ud_init(&ud);
	ud_set_mode(&ud, BITS_PER_LONG);
	ud_set_syntax(&ud, UD_SYN_INTEL);
	ud_set_vendor(&ud, UD_VENDOR_ANY);
	ud_set_input_buffer(&ud, entry, ud_get_stub_size(entry));

	while(ud_disassemble(&ud)){
		if(ud.mnemonic == UD_Imov && (ud.operand[1].type == UD_OP_IMM || ud.operand[1].type == UD_OP_MEM)){
			offset = ud_insn_len(&ud) - ptr_size;
			patch_addr = entry + ud_insn_off(&ud) + offset;

			if(offset > 0 && !memcmp(patch_addr, match, ptr_size)){
				//TODO Remove when done with stubs work. Also remove msgs.h from header
				procmon_info("MOV (offset: %d, type: %d): %s\n", offset, ud.operand[1].type, ud_insn_asm(&ud));
				memcpy(patch_addr, &addr, ptr_size);
				return;
			}
		}
	}
}

void *ud_find_syscall_table_addr(void *entry){
	ud_t ud;

	ud_init(&ud);
	ud_set_mode(&ud, BITS_PER_LONG);
	ud_set_vendor(&ud, UD_VENDOR_ANY);
	ud_set_input_buffer(&ud, entry, 512);

	while(ud_disassemble(&ud)){
		if(ud.mnemonic == UD_Icall && ud_insn_len(&ud) == 7){
#ifdef CONFIG_X86_32
			return (void *)ud.operand[0].lval.udword;
#elif defined(CONFIG_X86_64)
			return (void *)(0xffffffff00000000 | ud.operand[0].lval.udword);
#endif
		}
	}

	return NULL;
}