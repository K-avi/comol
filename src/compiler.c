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
//precedence of operators enum; 
//allows to know which one to write when on the bytecode and so on

typedef void (*ParseFn)(Scanner * scanner, Parser *parser); //watch out for this it's an easy source of mistakes

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;
//parser rule table for diferent operators

Chunk* compilingChunk; //oh hell no not the global compilingChunk 

static Chunk* currentChunk() {
  return compilingChunk;
}

static void errorAt(Token* token, const char* message, Parser * parser) {
    /*error reporting function ; 
    sets the parser into panic mode; 
    prints the line where the error occured on stderr W an error messages

    */
  if (parser->panicMode) return;
  parser->panicMode = true;
  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser->hadError = true;
}


static void error(const char* message, Parser * parser) {
    /*see error at*/
  errorAt(&parser->previous, message, parser);
}


static void errorAtCurrent(const char* message, Parser * parser) {
    /*see error at book fn*/
  errorAt(&parser->current, message, parser);
}


static void advanceParser(Scanner * scanner, Parser* parser) {
    /*book fn
    advances token being consumes  
    asks the scanner to generate a new token and breaks ; 

    reports an error if one is found
    
    */
  parser->previous = parser->current;

  for (;;) {
    parser->current = scanToken(scanner);
    if (parser->current.type != TOKEN_ERROR) break;

    errorAtCurrent(parser->current.start, parser);
  }
}

static void consume(TokenType type, const char* message, Scanner * scanner ,Parser * parser) {
    /* 
    book fn
    consumes a token of an expected type ; reports an error if one is found
    */
  if (parser->current.type == type) {
    advanceParser(scanner, parser);
    return;
  }

  errorAtCurrent(message, parser);
}

static void emitByte(uint8_t byte, Parser * parser) {
  /*
  writes a single byte in the current chunk at the current line
  */
  writeChunk(currentChunk(), byte, parser->current.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2, Parser * parser) {
  emitByte(byte1,parser);
  emitByte(byte2,parser);
}


static void emitReturn(Parser *parser) {
  emitByte(OP_RETURN, parser);
}





static void endCompiler(Parser * parser) {
  emitReturn(parser);
  #ifdef DEBUG_PRINT_CODE
  if (!parser->hadError) {
    disassembleChunk(currentChunk(), "code");
  }
#endif
}

//declare before cuz parser is recursive and so on
static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence, Scanner* scanner, Parser * parser);

static void parsePrecedence(Precedence precedence,Scanner* scanner, Parser *parser) {
  // really need to understand this 

  /*
  advances the parser ; sees if it can parse a prefix rule . 
  reports an error if it can't. 

  On succes; parses it.

  then enters a while loop to parse the infix expressions that the expression 
  being scanned is a part of 

  this allows to write the infix expression in correct order (somehow)
  */
  advanceParser(scanner, parser);
  ParseFn prefixRule = getRule(parser->previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expect expression.", parser);
    return;
  }

  prefixRule(scanner, parser);


  while (precedence <= getRule(parser->current.type)->precedence) {
    advanceParser(scanner, parser);
    ParseFn infixRule = getRule(parser->previous.type)->infix;
    infixRule(scanner, parser);
  }
}//need to understand better!!!


static void binary(Scanner * scanner, Parser* parser) {
  /*
  defines the rule to parse a binary op.

  first retrieves the operator from the parser 

  retrieves the function ptr to it's rule from the rule table 
  calls parse precedence 

  and writes the bytecode of the operation
  */
  TokenType operatorType = parser->previous.type;
  ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1), scanner, parser);

  switch (operatorType) {
    case TOKEN_PLUS:          emitByte(OP_ADD, parser); break;
    case TOKEN_MINUS:         emitByte(OP_SUBTRACT , parser); break;
    case TOKEN_STAR:          emitByte(OP_MULTIPLY, parser); break;
    case TOKEN_SLASH:         emitByte(OP_DIVIDE , parser); break;

    case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT, parser); break;
    case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL,parser); break;
    case TOKEN_GREATER:       emitByte(OP_GREATER, parser); break;
    case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT, parser); break;
    case TOKEN_LESS:          emitByte(OP_LESS, parser); break;
    case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT,parser ); break;
    default: return; // Unreachable.
  }
}


/*

would have to use 
static void ternary(Scanner * scanner, Parser* parser) {
  
  test for ternary op from challenges ; prolly won't be used
  
  TokenType operatorType = parser->previous.type;
  ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1), scanner, parser);
 parsePrecedence((Precedence)(rule->precedence + 1), scanner, parser);
  switch (operatorType) {
    case TOKEN_QMARK: emitByte( OP_QMARK, parser); break;

    default: return; // Unreachable.
  }
}
*/

static void expression(Scanner * scanner, Parser* parser) {
  // rule to parse an expression
  parsePrecedence(PREC_ASSIGNMENT, scanner, parser);
}


static void grouping(Scanner * scanner, Parser * parser) {
  /*
  rule to parse a grouping expression
  */
  expression(scanner, parser);
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.", scanner, parser);
}


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

static void number(Scanner* scanner, Parser * parser) {
  /*
  rule to parse a number litteral
  */
  double value = strtod(parser->previous.start, NULL);
  //emitConstant(value);

 // printf("prevline %d\n", parser.previous.line);
  writeConstant(currentChunk(), NUMBER_VAL(value), parser->previous.line); //using custom writeConstant fn
}



static void unary(Scanner* scanner , Parser * parser) {
  /*
  rule to parse an unary expression 
  (prefix minus or negation)
  */
  TokenType operatorType = parser->previous.type;

  // Compile the operand.
    parsePrecedence(PREC_UNARY, scanner, parser);

  // Emit the operator instruction.
  switch (operatorType) {
    case TOKEN_MINUS: emitByte(OP_NEGATE, parser); break;
     case TOKEN_BANG: emitByte(OP_NOT,parser); break;
    default: return; // Unreachable.
  }
}

static void literal(Scanner * scanner, Parser* parser) {
  switch (parser->previous.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE,parser); break;
    case TOKEN_NIL: emitByte(OP_NIL, parser); break;
    case TOKEN_TRUE: emitByte(OP_TRUE,parser ); break;
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
  [TOKEN_BANG]          = {unary,     NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};


static ParseRule* getRule(TokenType type) {
  /*
  retrieves the function pointer of the rule corresponding to a token type from the rules table constant
  */
  return &rules[type];
}

void initParser(Parser * parser){
  /*
  initialises a non null parser 
  */
  if(!parser) return;

  parser->current=(Token ){ 0, NULL, 0,0};
  parser->previous= (Token ){ 0, NULL, 0,0};
  parser->hadError=false;
  parser->panicMode=false;
}

bool compile(const char* source, Chunk* chunk){
  /*
  book fn
  */

 //printf("in compile source %p , chunk %p\n", (void*)source, (void*) chunk);
  if(! (source && chunk)){
    fprintf(stderr,"caught null chunk or source in compile at %p , source is %p, chunk is %p\n",(void*) &compile, (void*)&source, (void*) &chunk);
  }
  Scanner scanner;
  initScanner(source, &scanner);

  compilingChunk = chunk;
  
  
  
  Parser parser; 
  initParser(&parser);
 

  advanceParser(&scanner, &parser);
  expression(&scanner , &parser);
  consume(TOKEN_EOF, "Expect end of expression.", &scanner, &parser);

  endCompiler(&parser);
  return !parser.hadError;
}