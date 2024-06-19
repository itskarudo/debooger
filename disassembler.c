#include "disassembler.h"
#include "breakpoints.h"
#include "ui.h"
#include <capstone/capstone.h>
#include <stdint.h>
#include <sys/ptrace.h>

extern struct Breakpoint *breakpoints;

void disassemble(uint8_t *bytes, uint64_t pc) {
  csh handle;
  cs_insn *insn;
  size_t count;
  struct Breakpoint *bp, *tmp;

  cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
  count = cs_disasm(handle, bytes, DISASSEMBLY_LINES * MAX_INSTRUCTION_LENGTH,
                    pc, 0, &insn);
  if (count > 0) {
    size_t j;
    for (j = 0; j < DISASSEMBLY_LINES; j++) {
      if (insn[j].address == pc) {
        printf(BOLD(GREEN("►  0x%" PRIx64 "\t%s")) "\t\t" BLUE("%s") "\n",
               insn[j].address, insn[j].mnemonic, insn[j].op_str);
      } else {
        printf("   0x%" PRIx64 "\t" GREEN("%s") "\t\t" BLUE("%s") "\n",
               insn[j].address, insn[j].mnemonic, insn[j].op_str);
      }
    }

    cs_free(insn, count);
  } else
    printf("ERROR: Failed to disassemble given code!\n");

  cs_close(&handle);
}
