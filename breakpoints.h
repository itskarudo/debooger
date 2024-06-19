#pragma once

#include <stdbool.h>
#include <stdint.h>

void add_breakpoint(int pid, uint64_t address);
void remove_breakpoint(int pid, uint8_t id);
void list_breakpoints(int pid);
void enable_breakpoint(int pid, uint8_t id);
void disable_breakpoint(int pid, uint8_t id);
void free_breakpoints(int pid);
void apply_breakpoints(int pid);
