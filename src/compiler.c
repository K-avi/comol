#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
  Token current;
  Token previous;

  bool hadError;
  bool panicMode;
} Parser; //aaaaaaaaaaa

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Scanner * scanner);

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

Parser parser;


Chunk* compilingChunk; //oh hell no not the global compilingChunk 

static Chunk* currentChunk() {
  return compilingChunk;
}

static void errorAt(Token* token, const char* message) {
    /*book fn*/
  if (parser.panicMode) return;
  parser.panicMode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}


static void error(const char* message) {
    /*book fn*/
  errorAt(&parser.previous, message);
}


static void errorAtCurrent(const char* message) {
    /*book fn*/
  errorAt(&parser.current, message);
}


static void advanceParser(Scanner * scanner) {
    /*book fn*/
  parser.previous = parser.current;

  for (;;) {
    parser.current = scanToken(scanner);
    if (parser.current.type != TOKEN_ERROR) break;

    errorAtCurrent(parser.current.start);
  }
}

static void consume(TokenType type, const char* message, Scanner * scanner) {
    /* 
    book fn
    */
  if (parser.current.type == type) {
    advanceParser(scanner);
    return;
  }

  errorAtCurrent(message);
}

static void emitByte(uint8_t byte) {
  writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitReturn() {
  emitByte(OP_RETURN);
}




static void endCompiler() {
  emitReturn();
  #ifdef DEBUG_PRINT_CODE
  if (!parser.hadError) {
    disassembleChunk(currentChunk(), "code");
  }
#endif
}

//declare before cuz parser is recursive and so on
static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence, Scanner* scanner);

static void parsePrecedence(Precedence precedence,Scanner* scanner) {
  // really need to understand this 
  advanceParser(scanner);
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expect expression.");
    return;
  }

  prefixRule(scanner);


  while (precedence <= getRule(parser.current.type)->precedence) {
    advanceParser(scanner);
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    infixRule(scanner);
  }
}


static void binary(Scanner * scanner) {
  TokenType operatorType = parser.previous.type;
  ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1), scanner);

  switch (operatorType) {
    case TOKEN_PLUS:          emitByte(OP_ADD); break;
    case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
    case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
    case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
    default: return; // Unreachable.
  }
}

static void expression(Scanner * scanner) {
  // W
  parsePrecedence(PREC_ASSIGNMENT, scanner);
}


static void grouping(Scanner * scanner) {
  expression(scanner);
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.", scanner);
}


/*
static void emitBytes(uint8_t byte1, uint8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}
*/

/*
static uint8_t makeConstant(Value value) {
  int constant = addConstant(currentChunk(), value);
  if (constant > UINT8_MAX) {
    error("Too many constants in one chunk.");
    return 0;
  }

  return (uint8_t)constant;
}//longconstant !!!
*/
/*
static void emitConstant(Value value) {
  //book fn
  emitBytes(OP_CONSTANT, makeConstant(value));
}
*/

static void number(Scanner* scanner) {
  double value = strtod(parser.previous.start, NULL);
  //emitConstant(value);

  printf("prevline %d\n", parser.previous.line);
  writeConstant(currentChunk(), value, parser.previous.line); //using custom writeConstant fn
}



static void unary(Scanner* scanner) {
  TokenType operatorType = parser.previous.type;

  // Compile the operand.
    parsePrecedence(PREC_UNARY, scanner);

  // Emit the operator instruction.
  switch (operatorType) {
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    default: return; // Unreachable.
  }
}


ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};


static ParseRule* getRule(TokenType type) {
  return &rules[type];
}

bool compile(const char* source, Chunk* chunk){
  /*
  book fn
  */

 //printf("in compile source %p , chunk %p\n", (void*)source, (void*) chunk);
  if(! (source &&chunk)){
    fprintf(stderr,"caught null chunk or source in compile at %p , source is %p, chunk is %p\n",(void*) &compile, (void*)&source, (void*) &chunk);
  }
  Scanner scanner;
  initScanner(source, &scanner);
  
  compilingChunk = chunk;

  parser.hadError = false;
  parser.panicMode = false;

  advanceParser(&scanner);
  expression(&scanner);
  consume(TOKEN_EOF, "Expect end of expression.", &scanner);

   endCompiler();
  return !parser.hadError;
}