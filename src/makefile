TARGET := ../comol

CC := gcc
CFLAGS := -O2  -g -Wall -Wextra -Wpedantic -Wno-unused-parameter


SRCS := *.c

all: $(TARGET)

$(TARGET): $(SRCS) 
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(CLIBS)


clean:
	rm -f parser.tab.* parser.output lex.*.*  
	
.PHONY: clean