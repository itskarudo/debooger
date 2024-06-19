#pragma once
#include "parser.h"
#include <stdint.h>
#include <sys/user.h>

int64_t eval(struct Node *node, struct user_regs_struct *regs);
