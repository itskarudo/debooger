#include "lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const struct {
  char *name;
  enum Register reg;
} registers[];

void lexer_init(struct Lexer *lexer, char *source) {
  lexer->source = source;
  lexer->current = source;
}

void print_token(struct Token *token) {
  switch (token->type) {
#define __ENUMERATE_TOKEN(name)                                                \
  case name:                                                                   \
    printf(#name "\n");                                                        \
    break;
    ENUMERATE_TOKENS
#undef __ENUMERATE_TOKEN
  }
}

static void consume_whitespace(struct Lexer *lexer) {
  while (isspace(*lexer->current)) {
    lexer->current++;
  }
}

static inline bool match_literal_start(struct Lexer *lexer) {
  return isalpha(*lexer->current);
}

static inline bool match_literal_middle(struct Lexer *lexer) {
  return match_literal_start(lexer) || isdigit(*lexer->current) ||
         *lexer->current == '-';
}

struct Token next_token(struct Lexer *lexer) {
  consume_whitespace(lexer);

  if (*lexer->current == '\0') {
    return (struct Token){.type = TOK_EOL};
  }

  if (match_literal_start(lexer)) {
    char *start = lexer->current;
    while (match_literal_middle(lexer)) {
      lexer->current++;
    }
    return (struct Token){.type = TOK_LITERAL,
                          .value.as_literal = {start, lexer->current - start}};
  }

  if (*lexer->current == '$') {
    lexer->current++;
    char *start = lexer->current;

    while (isalnum(*lexer->current)) {
      lexer->current++;
    }

    for (size_t i = 0; i < REGISTERS_COUNT; i++) {
      if (strncmp(start, registers[i].name, lexer->current - start) == 0) {
        return (struct Token){.type = TOK_REGISTER,
                              .value.as_register = registers[i].reg};
      }
    }

    return (struct Token){.type = TOK_INVALID};
  }

  if (isdigit(*lexer->current)) {

    if (*lexer->current == '0' && lexer->current[1] == 'x') {
      lexer->current += 2;
      char *start = lexer->current;
      while (isxdigit(*lexer->current)) {
        lexer->current++;
      }
      return (struct Token){.type = TOK_NUMBER,
                            .value.as_number = strtoull(start, NULL, 16)};
    }

    char *start = lexer->current;
    while (isdigit(*lexer->current)) {
      lexer->current++;
    }
    return (struct Token){.type = TOK_NUMBER,
                          .value.as_number = strtoull(start, NULL, 0)};
  }

  if (*lexer->current == '+') {
    lexer->current++;
    return (struct Token){.type = TOK_ADD};
  }

  if (*lexer->current == '-') {
    lexer->current++;
    return (struct Token){.type = TOK_SUB};
  }

  if (*lexer->current == '*') {
    lexer->current++;
    return (struct Token){.type = TOK_MUL};
  }

  if (*lexer->current == '/') {
    lexer->current++;
    return (struct Token){.type = TOK_DIV};
  }

  if (*lexer->current == '(') {
    lexer->current++;
    return (struct Token){.type = TOK_LPAREN};
  }

  if (*lexer->current == ')') {
    lexer->current++;
    return (struct Token){.type = TOK_RPAREN};
  }

  return (struct Token){.type = TOK_INVALID};
}
