#pragma once

#include "lexer.h"
#include <stdint.h>
#include <sys/user.h>

enum NodeType {
  NODE_NUMBER,
  NODE_REGISTER,
  NODE_ADD = TOK_ADD,
  NODE_MUL = TOK_MUL,
  NODE_SUB = TOK_SUB,
  NODE_DIV = TOK_DIV,
};

struct Node {
  enum NodeType type;
  union {
    int64_t as_number;
    enum Register as_register;
    struct {
      struct Node *lhs;
      struct Node *rhs;
    } as_bi_op;
  } value;
};

struct CommandInstance parse_cmd(char *line);
void free_node(struct Node *node);
