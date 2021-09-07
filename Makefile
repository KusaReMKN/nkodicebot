SHELL	= /bin/sh
CC	= gcc
CFLAGS	= -O2 -Wall

PROGRAM	= nkobot
OBJS	= nkobot.o mknmap.o

.PHONY: all
all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $^

.PHONY: clean
clean:
	$(RM) $(PROGRAM) $(OBJS)
