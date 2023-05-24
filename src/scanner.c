#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"


//every function relies on the global scanner 

//Scanner scanner; //change to not use global at some point

void initScanner(const char* source, Scanner *scanner) {
  /*
  from book
  initialises the global scanner ; doesn't check if source is null
  */
  scanner->start = source;
  scanner->current = source;
  scanner->line = 1;
}

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
          c == '_';
}

static bool isDigit(char c) {//isn't isDigit already a thing?
  return c >= '0' && c <= '9';
}


static inline bool isAtEnd(Scanner * scanner) {
    //book fn
  return *scanner->current == '\0';
}
static char advanceScanner(Scanner * scanner) {
    //book fn
  scanner->current++;
  return *(scanner->current-1);
}

static bool match(char expected, Scanner * scanner) {//book fn 
  /*
  returns true if the next char in the scanner == expected false otherwise; 
  increments the current char if the return val is true
  */
  if (isAtEnd(scanner)) return false;
  if (*scanner->current != expected) return false;
  scanner->current++;
  return true;
}

static Token makeToken(TokenType type, Scanner * scanner) {
  /*
  simple token builder 
  */
  Token token;
  token.type = type;
  token.start = scanner->start;
  token.length = (int)(scanner->current - scanner->start);
  token.line = scanner->line;
  return token;
}

static Token errorToken(const char* message,Scanner* scanner) {
  /*
  from book
  similar to make token except the start of the token is the error message
  */
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strnlen(message,256);
  token.line = scanner->line;
  return token;
}

static char peek(Scanner* scanner) {
    //book fn
  return *scanner->current;
}

static char peekNext(Scanner* scanner) {//book fn
  if (isAtEnd(scanner)) return '\0';
  return scanner->current[1];
}

static void skipWhitespace(Scanner * scanner) {
    //book fn ; consumes every whitespace and comment
  for (;;) {
    char c = peek(scanner);
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advanceScanner(scanner);
        break;
      case '\n':
        (scanner->line)++;
        advanceScanner(scanner);
        break;
      case '/':
        if (peekNext(scanner) == '/') {
          // A comment goes until the end of the line.
          while (peek(scanner) != '\n' && !isAtEnd(scanner)) advanceScanner(scanner);
        } else {
          return;
        }
        break;
      default:
        return;
    }
  }
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type, Scanner* scanner) {
 //book fn
 /*
 checks if the current token being scanned is a reserved keyword or a valid identifier
 returns the token_type if matched identifier otherwise
 */
  if (scanner->current-scanner->start == start+length && memcmp(scanner->start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}

static TokenType identifierType(Scanner * scanner) {
    /*book fn*/
  /*
  checks if the current token being scanned is an identifier or a reserved keyword; 
  and returns the corresponding token ; 

  don't like it's implementation ; will prolly do the big DFA by hand at some point
  */
  switch (scanner->start[0]) {
    case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND, scanner);
    case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS , scanner);
    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE, scanner);
    case 'i': return checkKeyword(1, 1, "f", TOKEN_IF, scanner);
    case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL, scanner);
    case 'o': return checkKeyword(1, 1, "r", TOKEN_OR, scanner);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT, scanner);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN, scanner);
    case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER, scanner);
    case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR, scanner);
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE, scanner);
    case 'f':
      if (scanner->current - scanner->start > 1) {
        switch (scanner->start[1]) {
          case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE, scanner);
          case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR, scanner);
          case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN, scanner);
        }
      }
      break;
  }
  return TOKEN_IDENTIFIER;
}

static Token string(Scanner * scanner) {
  /*
  consumes characters until finding a " to close a string 
  returns the token corresponding to the string literal consumed
  */
  while (peek(scanner) != '"' && !isAtEnd(scanner)) {
    if (peek(scanner) == '\n') (scanner->line)++;
    advanceScanner(scanner);
  }

  if (isAtEnd(scanner)) return errorToken("Unterminated string.",scanner);

  // The closing quote.
  advanceScanner(scanner);
  return makeToken(TOKEN_STRING, scanner);
}

static Token number(Scanner * scanner) {
    /*
    consumes number characters and returns the token corresponding to it
    */
  while (isDigit(peek(scanner))) advanceScanner(scanner);

  // Look for a fractional part.
  if (peek(scanner) == '.' && isDigit(peekNext(scanner))) {
    // Consume the ".".
    advanceScanner(scanner);

    while (isDigit(peek(scanner))) advanceScanner(scanner);
  }

  return makeToken(TOKEN_NUMBER, scanner);
}

static Token identifier(Scanner* scanner) {
  while (isAlpha(peek(scanner)) || isDigit(peek(scanner))) advanceScanner(scanner);
  return makeToken(identifierType(scanner), scanner);
}

Token scanToken(Scanner * scanner) {
  /*
  beeg scanner function  from the book ; will have to make it fit the 
  COMOL lexic at some point

  scans 1 token from the scanner singleton and returns it
  */

  skipWhitespace(scanner);

  scanner->start = scanner->current;

  if (isAtEnd(scanner)) return makeToken(TOKEN_EOF, scanner);


  char c = advanceScanner(scanner);
  if (isAlpha(c)) return identifier(scanner);
  if (isDigit(c)) return number(scanner);

  switch (c) {
    case '(': return makeToken(TOKEN_LEFT_PAREN , scanner);
    case ')': return makeToken(TOKEN_RIGHT_PAREN, scanner);
    case '{': return makeToken(TOKEN_LEFT_BRACE, scanner);
    case '}': return makeToken(TOKEN_RIGHT_BRACE, scanner);
    case ';': return makeToken(TOKEN_SEMICOLON, scanner);
    case ',': return makeToken(TOKEN_COMMA, scanner);
    case '.': return makeToken(TOKEN_DOT, scanner);
    case '-': return makeToken(TOKEN_MINUS, scanner);
    case '+': return makeToken(TOKEN_PLUS, scanner);
    case '/': return makeToken(TOKEN_SLASH, scanner);
    case '*': return makeToken(TOKEN_STAR, scanner);

    case '!':
      return makeToken(
          match('=',scanner) ? TOKEN_BANG_EQUAL : TOKEN_BANG, scanner);
    case '=':
      return makeToken(
          match('=', scanner) ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL, scanner);
    case '<':
      return makeToken(
          match('=',scanner) ? TOKEN_LESS_EQUAL : TOKEN_LESS, scanner);
    case '>':
      return makeToken(
          match('=',scanner) ? TOKEN_GREATER_EQUAL : TOKEN_GREATER, scanner);
    case '"': return string(scanner);
  }

  return errorToken("Unexpected character.", scanner);
}