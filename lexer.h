#pragma once

#include <stddef.h>
#include <stdint.h>

enum Register : uint8_t {
  REG_R15,
  REG_R14,
  REG_R13,
  REG_R12,
  REG_RBP,
  REG_RBX,
  REG_R11,
  REG_R10,
  REG_R9,
  REG_R8,
  REG_RAX,
  REG_RCX,
  REG_RDX,
  REG_RSI,
  REG_RDI,
  REG_ORIG_RAX,
  REG_RIP,
  REG_CS,
  REG_EFLAGS,
  REG_RSP,
  REG_SS,
  REG_FS_BASE,
  REG_GS_BASE,
  REG_DS,
  REG_ES,
  REG_FS,
  REG_GS,
  REGISTERS_COUNT
};

#define ENUMERATE_TOKENS                                                       \
  __ENUMERATE_TOKEN(TOK_LITERAL)                                               \
  __ENUMERATE_TOKEN(TOK_NUMBER)                                                \
  __ENUMERATE_TOKEN(TOK_REGISTER)                                              \
  __ENUMERATE_TOKEN(TOK_ADD)                                                   \
  __ENUMERATE_TOKEN(TOK_MUL)                                                   \
  __ENUMERATE_TOKEN(TOK_SUB)                                                   \
  __ENUMERATE_TOKEN(TOK_DIV)                                                   \
  __ENUMERATE_TOKEN(TOK_LPAREN)                                                \
  __ENUMERATE_TOKEN(TOK_RPAREN)                                                \
  __ENUMERATE_TOKEN(TOK_EOL)                                                   \
  __ENUMERATE_TOKEN(TOK_INVALID)

enum TokenType : uint8_t {
#define __ENUMERATE_TOKEN(token) token,
  ENUMERATE_TOKENS
#undef __ENUMERATE_TOKEN
};

struct Token {
  enum TokenType type;
  union {
    int64_t as_number;
    enum Register as_register;
    struct {
      const char *start;
      size_t length;
    } as_literal;
  } value;
};

struct Lexer {
  char *source;
  char *current;
};

void lexer_init(struct Lexer *lexer, char *source);
struct Token next_token(struct Lexer *lexer);
void print_token(struct Token *token);
