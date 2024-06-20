#include "breakpoints.h"
#include "commands.h"
#include "disassembler.h"
#include "eval.h"
#include "lexer.h"
#include "parser.h"
#include "ui.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/personality.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#define STACK_LINES 10

int pid;
struct user_regs_struct regs;

static struct winsize w;

const struct {
  char *name;
  enum Register reg;
} registers[] = {
    {"r15", REG_R15},         {"r14", REG_R14},
    {"r13", REG_R13},         {"r12", REG_R12},
    {"rbp", REG_RBP},         {"rbx", REG_RBX},
    {"r11", REG_R11},         {"r10", REG_R10},
    {"r9", REG_R9},           {"r8", REG_R8},
    {"rax", REG_RAX},         {"rcx", REG_RCX},
    {"rdx", REG_RDX},         {"rsi", REG_RSI},
    {"rdi", REG_RDI},         {"orig_rax", REG_ORIG_RAX},
    {"rip", REG_RIP},         {"cs", REG_CS},
    {"eflags", REG_EFLAGS},   {"rsp", REG_RSP},
    {"ss", REG_SS},           {"fs_base", REG_FS_BASE},
    {"gs_base", REG_GS_BASE}, {"ds", REG_DS},
    {"es", REG_ES},           {"fs", REG_FS},
    {"gs", REG_GS},
};

void draw_separator() {
  printf("\033[34m");
  for (size_t i = 0; i < w.ws_col; i++) {
    printf("─");
  }
  printf("\033[0m\n");
}

void draw_titled_separator(const char *title) {
  int len = strlen(title) + 4;
  int side = (w.ws_col - len) / 2;

  printf("\033[34m");
  for (size_t i = 0; i < side; i++) {
    printf("─");
  }
  printf("[ %s ]", title);
  for (size_t i = 0; i < side; i++) {
    printf("─");
  }
  printf("\033[0m\n");
}

void run_tracee(char *argv[]) {
  personality(ADDR_NO_RANDOMIZE);
  ptrace(PTRACE_TRACEME, 0, 0, 0);
  execve(argv[0], argv, NULL);
}

struct CommandInstance read_command() {
  static char *prev_line = NULL;
  char *line = NULL;

  size_t len = 0;

  len = getline(&line, &len, stdin);

  bool is_empty = true;
  for (size_t i = 0; i < len; i++) {
    if (!isspace(line[i])) {
      is_empty = false;
      break;
    }
  }

  if (is_empty) {
    free(line);
    line = prev_line;
  } else {
    if (prev_line != NULL)
      free(prev_line);
    prev_line = line;
  }

  return parse_cmd(line);
}

void run_tracer() {
  uint8_t instructions_buffer[DISASSEMBLY_LINES * MAX_INSTRUCTION_LENGTH];

  int status;

  while (1) {

    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
      puts("exited.");
      break;
    }

    ioctl(0, TIOCGWINSZ, &w);

    draw_titled_separator("REGISTERS");

    ptrace(PTRACE_GETREGS, pid, 0, &regs);

    for (size_t i = 0; i < REGISTERS_COUNT; i++) {
      printf("   " BOLD("%s") "\t0x%lx\n", registers[i].name,
             *((uint64_t *)(&regs) + registers[i].reg));
    }

    draw_titled_separator("DISASSEMBLY");

    for (size_t i = 0; i < DISASSEMBLY_LINES * MAX_INSTRUCTION_LENGTH; i += 8) {
      uint64_t val = ptrace(PTRACE_PEEKDATA, pid, regs.rip + i, 0);
      memcpy(instructions_buffer + i, &val, 8);
    }

    disassemble(instructions_buffer, regs.rip);

    draw_titled_separator("STACK");

    for (int i = 0; i < STACK_LINES; i++) {
      uint64_t value = ptrace(PTRACE_PEEKDATA, pid, regs.rsp + i * 8, 0);
      printf("   " YELLOW("0x%llx") " —▸ 0x%lx\n", regs.rsp + i * 8, value);
    }

    draw_separator();

    enum ExecState exec_state = PAUSE_EXEC;

    while (exec_state == PAUSE_EXEC) {
      printf(BOLD(RED("[0x%llx]> ")), regs.rip);
      struct CommandInstance instance = read_command();

      if (instance.cmd == NULL) {
        puts("invalid command.");
        continue;
      }

      int64_t value = 0;
      if (instance.cmd->takes_arg) {
        if (instance.arg == NULL) {
          puts("missing argument.");
          continue;
        }

        value = eval(instance.arg, &regs);
        if (instance.cmd->takes_arg)
          free_node(instance.arg);
      }

      exec_state = instance.cmd->handler(pid, value);
    }

    if (exec_state == EXIT_EXEC) {
      goto _cleanup;
    }
  }
_cleanup:
  free_breakpoints(pid);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s exec-file\n", argv[0]);
    exit(1);
  }

  pid = fork();
  if (pid == 0)
    run_tracee(&argv[1]);
  else
    run_tracer();
}
