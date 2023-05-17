TARGET := ../comol

CC := gcc
CFLAGS := -O2  -g -Wall -Wextra -Wpedantic -Wno-unused-parameter


SRCS := debug.c chunk.c memory.c value.c

all: $(TARGET)

$(TARGET): $(SRCS) main.c
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(CLIBS)


clean:
	rm -f parser.tab.* parser.output lex.*.*  
	
.PHONY: clean