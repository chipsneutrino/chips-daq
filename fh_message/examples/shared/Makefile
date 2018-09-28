
CC=gcc

FH_MESSAGE_DIR = ../..
INCDIR=$(FH_MESSAGE_DIR)/include

# _GNU_SOURCE defined to bring in support for pthread muttex attribute support
CFLAGS=-I$(INCDIR) -O2 -std=c99 -Wall -Werror -pedantic -D_GNU_SOURCE
AR=ar

LIBOUT=libfh_example.a

SRC = $(wildcard *.c)
OBJS = $(SRC:.c=.o)

%.o : %.c %.h
	$(CC) -c -I. $(CFLAGS) $<


all: $(LIBOUT)

$(LIBOUT): $(OBJS)
	$(AR) rcs $@ $(OBJS)

.PHONY: clean

clean:
	rm -f *.o $(LIBOUT)
