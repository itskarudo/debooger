#pragma once

#include <stdbool.h>
#include <stdint.h>

enum ExecState : uint8_t { CONTINUE_EXEC, PAUSE_EXEC, EXIT_EXEC };

// TODO: args should probably be variadic
typedef enum ExecState (*cmd_handler_t)(int pid, int64_t value);

struct Command {
  const char *alias;
  cmd_handler_t handler;
  bool takes_arg;
};

struct CommandInstance {
  struct Command *cmd;
  struct Node *arg;
};
