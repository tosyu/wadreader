PROGRAM=wadreader
SOURCES=./src/main.c ./src/wadutils.c

CC=gcc
CFLAGS=-g -Wall -Werror -Wextra -ansi
INCLUDES=-I./src -I/usr/include
LDFLAGS=

OBJECTS=$(SOURCES:%.c=%.o)

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm ${OBJECTS} $(PROGRAM)

