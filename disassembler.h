#pragma once

#include <stdint.h>

#define DISASSEMBLY_LINES 16
#define MAX_INSTRUCTION_LENGTH 15

void disassemble(uint8_t *bytes, uint64_t pc);
