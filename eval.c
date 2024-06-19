#include "parser.h"

int64_t eval(struct Node *node, struct user_regs_struct *regs) {

  if (node->type == NODE_NUMBER) {
    return node->value.as_number;
  }

  if (node->type == NODE_REGISTER) {
    return *((uint64_t *)regs + node->value.as_register);
  }

  if (node->type == NODE_ADD) {
    return eval(node->value.as_bi_op.lhs, regs) +
           eval(node->value.as_bi_op.rhs, regs);
  }

  if (node->type == NODE_SUB) {
    return eval(node->value.as_bi_op.lhs, regs) -
           eval(node->value.as_bi_op.rhs, regs);
  }

  if (node->type == NODE_MUL) {
    return eval(node->value.as_bi_op.lhs, regs) *
           eval(node->value.as_bi_op.rhs, regs);
  }

  if (node->type == NODE_DIV) {
    return eval(node->value.as_bi_op.lhs, regs) /
           eval(node->value.as_bi_op.rhs, regs);
  }

  return -1;
}
