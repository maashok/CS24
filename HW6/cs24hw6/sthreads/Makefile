#
# Commands and arguments
#
CC = gcc
RM = rm

# ADD -m32 ON 64BIT PLATFORMS
CFLAGS = -Wall -g -std=c99 -pedantic

# ADD -32 ON 64BIT PLATFORMS
ASFLAGS = -g


all: test testarg testret


# The simple test program
test: sthread.o glue.o test.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

testarg: sthread.o glue.o test_arg.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

testret: sthread.o glue.o test_ret.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# pseudo-target to clean up
clean:
	$(RM) -f *.o core* *~ test testret testarg


.PHONY: all clean


# Dependencies
sthread.c: sthread.h
test.c: sthread.h

