#include "breakpoints.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/user.h>

struct Breakpoint {
  uint64_t address;
  bool is_enabled;
};

enum DebugReg : uint8_t {
  DR0 = 0,
  DR1,
  DR2,
  DR3,
  DR4,
  DR5,
  DR6,
  DR7,
};

#define MAX_BREAKPOINTS 4

static struct Breakpoint *breakpoints[MAX_BREAKPOINTS];

static struct Breakpoint **find_free_slot(void) {
  for (size_t i = 0; i < MAX_BREAKPOINTS; i++) {
    if (breakpoints[i] == NULL) {
      return &breakpoints[i];
    }
  }
  return NULL;
}

void add_breakpoint(int pid, uint64_t address) {
  struct Breakpoint **bpp = find_free_slot();
  if (!bpp) {
    puts("? No free breakpoint slots.");
    return;
  }

  *bpp = malloc(sizeof(struct Breakpoint));
  assert(*bpp);

  struct Breakpoint *bp = *bpp;

  bp->address = address;
  bp->is_enabled = true;
}

void remove_breakpoint(int pid, uint8_t id) {
  assert(id < MAX_BREAKPOINTS);
  assert(breakpoints[id]);

  free(breakpoints[id]);
  breakpoints[id] = NULL;
}

void list_breakpoints(int pid) {
  for (size_t i = 0; i < MAX_BREAKPOINTS; i++) {
    if (!breakpoints[i])
      continue;
    printf("Breakpoint #%zu: %p (%s)\n", i, (void *)breakpoints[i]->address,
           breakpoints[i]->is_enabled ? "enabled" : "disabled");
  }
}

void enable_breakpoint(int pid, uint8_t id) {

  if (id >= MAX_BREAKPOINTS || !breakpoints[id]) {
    puts("? Invalid breakpoint ID.");
    return;
  }

  breakpoints[id]->is_enabled = true;
}

void disable_breakpoint(int pid, uint8_t id) {

  if (id >= MAX_BREAKPOINTS || !breakpoints[id]) {
    puts("? Invalid breakpoint ID.");
    return;
  }

  breakpoints[id]->is_enabled = false;
}

void free_breakpoints(int pid) {
  for (size_t i = 0; i < MAX_BREAKPOINTS; i++) {
    if (!breakpoints[i])
      continue;

    free(breakpoints[i]);
    breakpoints[i] = NULL;
  }
}

const uint8_t DR7_LEN_BIT[] = {19, 23, 27, 31};
const uint8_t DR7_RW_BIT[] = {17, 21, 25, 29};
const uint8_t DR7_LE_BIT[] = {0, 2, 4, 6};
const uint8_t DR7_GE_BIT[] = {1, 3, 5, 7};

#define DR7_LEN_SIZE 2
#define DR7_RW_SIZE 2

#define DR_OFFSET(dr) ((((struct user *)0)->u_debugreg) + (dr))

static inline uint64_t get_reg(int pid, enum DebugReg reg) {
  return ptrace(PTRACE_PEEKUSER, pid, DR_OFFSET(reg), 0);
}

static inline void set_reg(int pid, enum DebugReg reg, uint64_t value) {
  ptrace(PTRACE_POKEUSER, pid, DR_OFFSET(reg), value);
}

static inline void set_bit(uint32_t *val, uint32_t set_val, uint8_t start_bit,
                           uint8_t bit_count) {
  uint32_t mask = 0xFFFFFFFF << (32 - bit_count);
  mask = mask >> (31 - start_bit);
  uint32_t inv_mask = ~mask;

  *val = *val & inv_mask;
  *val = *val | (set_val << (start_bit + 1 - bit_count));
}

static inline bool get_bit(uint32_t val, uint8_t bit_index) {
  uint32_t mask = 1 << bit_index;
  uint32_t masked_val = val & mask;
  return masked_val != 0;
}

void apply_breakpoints(int pid) {
  uint32_t dr7 = get_reg(pid, DR7);

  for (size_t i = 0; i < MAX_BREAKPOINTS; i++) {

    if (!breakpoints[i] || !breakpoints[i]->is_enabled) {
      set_bit(&dr7, 0, DR7_LE_BIT[i], 1);
      continue;
    }

    struct Breakpoint *bp = breakpoints[i];

    set_bit(&dr7, 0, DR7_LEN_BIT[i], DR7_LEN_SIZE);
    set_bit(&dr7, 0, DR7_RW_BIT[i], DR7_LEN_SIZE);
    set_bit(&dr7, 1, DR7_LE_BIT[i], 1);

    switch (i) {
    case 0:
      set_reg(pid, DR0, bp->address);
      break;
    case 1:
      set_reg(pid, DR1, bp->address);
      break;
    case 2:
      set_reg(pid, DR2, bp->address);
      break;
    case 3:
      set_reg(pid, DR3, bp->address);
      break;
    }
  }

  set_reg(pid, DR7, dr7);
}
