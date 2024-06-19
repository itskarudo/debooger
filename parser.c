#include "parser.h"
#include "commands.h"
#include "lexer.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern struct Command commands[];

enum OperatorType : uint8_t {
  OP_ADD = TOK_ADD,
  OP_SUB = TOK_SUB,
  OP_MUL = TOK_MUL,
  OP_DIV = TOK_DIV,
  OP_LPAREN = TOK_LPAREN,
  OP_RPAREN = TOK_RPAREN,
};

struct Operator {
  enum OperatorType op;
  uint8_t precedence;
};

static struct Operator operators[] = {
    {OP_ADD, 1}, {OP_SUB, 1},    {OP_MUL, 2},
    {OP_DIV, 2}, {OP_LPAREN, 0}, {OP_RPAREN, 0},
};

static inline struct Operator *is_operator(struct Token *token) {
  for (size_t i = 0; i < sizeof(operators) / sizeof(operators[0]); i++) {
    if ((enum TokenType)operators[i].op == token->type) {
      return &operators[i];
    }
  }
  return NULL;
}

#define TRY(expr)                                                              \
  ({                                                                           \
    typeof(expr) _res = expr;                                                  \
    if (!_res) {                                                               \
      value_stack_cur = 0;                                                     \
      operator_stack_cur = 0;                                                  \
      return NULL;                                                             \
    }                                                                          \
    _res;                                                                      \
  })

#define VALUE_STACK_MAX 256
#define OPERATOR_STACK_MAX 256

static struct Node *value_stack[VALUE_STACK_MAX];
static size_t value_stack_cur = 0;

static inline void push_value(struct Node *node) {
  assert(value_stack_cur < VALUE_STACK_MAX);
  value_stack[value_stack_cur++] = node;
}

static inline struct Node *pop_value() {
  if (value_stack_cur <= 0)
    return NULL;
  return value_stack[--value_stack_cur];
}

static struct Operator *operator_stack[OPERATOR_STACK_MAX];
static size_t operator_stack_cur = 0;

static inline void push_operator(struct Operator *op) {
  assert(operator_stack_cur < OPERATOR_STACK_MAX);
  operator_stack[operator_stack_cur++] = op;
}

static inline struct Operator *pop_operator() {
  if (operator_stack_cur <= 0)
    return NULL;
  return operator_stack[--operator_stack_cur];
}

static struct Node *parse_expr(struct Lexer *lexer) {

  while (1) {
    struct Token token = next_token(lexer);
    if (token.type == TOK_EOL)
      break;

    if (token.type == TOK_LITERAL || token.type == TOK_INVALID) {
      return NULL;
    }

    if (token.type == TOK_NUMBER) {
      struct Node *node = malloc(sizeof(struct Node));
      node->type = NODE_NUMBER;
      node->value.as_number = token.value.as_number;
      push_value(node);
      continue;
    }

    if (token.type == TOK_REGISTER) {
      struct Node *node = malloc(sizeof(struct Node));
      node->type = NODE_REGISTER;
      node->value.as_register = token.value.as_register;
      push_value(node);
      continue;
    }

    struct Operator *op;

    if ((op = is_operator(&token))) {
      if (op->op == OP_LPAREN) {
        push_operator(op);
        continue;
      }

      if (op->op == OP_RPAREN) {
        while (operator_stack_cur > 0) {

          struct Operator *top_op = TRY(pop_operator());
          if (top_op->op == OP_LPAREN)
            break;

          struct Node *node = malloc(sizeof(struct Node));
          node->type = (enum NodeType)top_op->op;

          node->value.as_bi_op.rhs = TRY(pop_value());
          node->value.as_bi_op.lhs = TRY(pop_value());

          push_value(node);
        }
        continue;
      }

      while (operator_stack_cur > 0) {

        struct Operator *top_op = TRY(pop_operator());
        if (top_op->precedence < op->precedence)
          break;

        struct Node *node = malloc(sizeof(struct Node));
        node->type = (enum NodeType)top_op->op;

        node->value.as_bi_op.rhs = TRY(pop_value());
        node->value.as_bi_op.lhs = TRY(pop_value());

        push_value(node);
      }

      push_operator(op);
      continue;
    }
  }

  while (operator_stack_cur > 0) {
    struct Node *node = malloc(sizeof(struct Node));
    node->type = (enum NodeType)TRY(pop_operator())->op;

    node->value.as_bi_op.rhs = TRY(pop_value());
    node->value.as_bi_op.lhs = TRY(pop_value());

    push_value(node);
  }

  return pop_value();
}

void free_node(struct Node *node) {
  if (node->type == NODE_NUMBER || node->type == NODE_REGISTER) {
    free(node);
    return;
  }
  free_node(node->value.as_bi_op.lhs);
  free_node(node->value.as_bi_op.rhs);
  free(node);
}

struct CommandInstance parse_cmd(char *const line) {

  struct CommandInstance instance = {.cmd = NULL};
  struct Token token;

  if (line == NULL)
    return instance;

  struct Lexer lexer;
  lexer_init(&lexer, line);

  token = next_token(&lexer);

  if (token.type != TOK_LITERAL) {
    return instance;
  }

  for (size_t i = 0; commands[i].handler; i++) {
    if (strncmp(token.value.as_literal.start, commands[i].alias,
                token.value.as_literal.length) == 0) {

      instance.cmd = &commands[i];

      if (commands[i].takes_arg)
        instance.arg = parse_expr(&lexer);

      return instance;
    }
  }

  return instance;
}
