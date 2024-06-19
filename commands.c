#include "commands.h"
#include "breakpoints.h"
#include <stddef.h>
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/user.h>

static enum ExecState cmd_stepinto(int pid, int64_t value) {
  (void)value;
  ptrace(PTRACE_SINGLESTEP, pid, 0, 0);
  return CONTINUE_EXEC;
}

static enum ExecState cmd_go(int pid, int64_t value) {
  (void)value;
  apply_breakpoints(pid);
  ptrace(PTRACE_SYSCALL, pid, 0, 0);
  return CONTINUE_EXEC;
}

static enum ExecState cmd_continue(int pid, int64_t value) {
  (void)value;
  apply_breakpoints(pid);
  ptrace(PTRACE_CONT, pid, 0, 0);
  return CONTINUE_EXEC;
}

static enum ExecState cmd_quit(int pid, int64_t value) {
  (void)value;
  (void)pid;
  return EXIT_EXEC;
}

static enum ExecState cmd_eval(int pid, int64_t value) {
  (void)pid;
  printf("?: 0x%lx\n", value);
  return PAUSE_EXEC;
}

static enum ExecState cmd_examine(int pid, int64_t value) {
  (void)pid;
  uint64_t content_raw = ptrace(PTRACE_PEEKDATA, pid, value, 0);
  uint8_t *content = (uint8_t *)&content_raw;
  printf("x: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
         content[0], content[1], content[2], content[3], content[4], content[5],
         content[6], content[7]);

  return PAUSE_EXEC;
}

static enum ExecState cmd_pid(int pid, int64_t value) {
  (void)value;
  printf("%d\n", pid);
  return PAUSE_EXEC;
}

static enum ExecState cmd_break(int pid, int64_t value) {
  add_breakpoint(pid, value);
  return PAUSE_EXEC;
}

static enum ExecState cmd_remove_breakpoint(int pid, int64_t value) {
  remove_breakpoint(pid, value);
  return PAUSE_EXEC;
}

static enum ExecState cmd_list_breakpoints(int pid, int64_t value) {
  (void)value;
  list_breakpoints(pid);
  return PAUSE_EXEC;
}

static enum ExecState cmd_enable_breakpoint(int pid, int64_t value) {
  enable_breakpoint(pid, value);
  return PAUSE_EXEC;
}

static enum ExecState cmd_disable_breakpoint(int pid, int64_t value) {
  disable_breakpoint(pid, value);
  return PAUSE_EXEC;
}

struct Command commands[] = {{"s", cmd_stepinto, false},
                             {"g", cmd_go, false},
                             {"c", cmd_continue, false},
                             {"q", cmd_quit, false},
                             {"e", cmd_eval, true},
                             {"x", cmd_examine, true},
                             {"pid", cmd_pid, false},
                             {"b", cmd_break, true},
                             {"br", cmd_remove_breakpoint, true},
                             {"bl", cmd_list_breakpoints, false},
                             {"be", cmd_enable_breakpoint, true},
                             {"bd", cmd_disable_breakpoint, true},
                             {NULL, NULL, false}};
