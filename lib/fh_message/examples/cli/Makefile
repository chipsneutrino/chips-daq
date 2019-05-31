
CC=gcc
FH_MESSAGE_DIR =../..
FH_EXAMPLE_DIR =..

INCDIRS=$(FH_MESSAGE_DIR)/include $(FH_MESSAGE_DIR)/include/udaq_api
LIBDIRS = $(FH_MESSAGE_DIR)/src
LIBS = fh_message

CFLAGS=$(INCDIRS:%=-I%) -O2 -std=c99 -Wall -Werror -pedantic

BIN=cli


DEPS = $(FH_MESSAGE_DIR)/src/libfh_message.a cli.c
OBJS = 

%.o : %.c %.h $(DEPS)
	$(CC) -c -I. $(CFLAGS) $<


all: $(BIN)

cli: $(OBJS) cli.o $(DEPS)
	$(CC) $(CFLAGS) $(LIBDIRS:%=-L%) -o $@  $(OBJS) $@.o $(LIBS:%=-l%)

.PHONY: clean

clean:
	rm -f *.o $(BIN)
