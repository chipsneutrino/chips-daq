
CC=gcc
FH_MESSAGE_DIR =../..
FH_EXAMPLE_DIR =..

INCDIRS=$(FH_MESSAGE_DIR)/include $(FH_MESSAGE_DIR)/include/udaq_api
LIBDIRS = $(FH_MESSAGE_DIR)/src
LIBS = fh_message

CFLAGS=$(INCDIRS:%=-I%) -O2 -std=c99 -Wall -Werror -pedantic

BIN=translator

DEPS = $(FH_MESSAGE_DIR)/src/libfh_message.a translator.c msg_dictionary.c
OBJS = msg_dictionary.o

%.o : %.c %.h $(DEPS)
	$(CC) -c -I. $(CFLAGS) $<

all: $(BIN)

translator: $(OBJS) translator.o $(DEPS)
	$(CC) $(CFLAGS) $(LIBDIRS:%=-L%) -o $@  $(OBJS) $@.o $(LIBS:%=-l%)

.PHONY: clean

clean:
	rm -f *.o $(BIN)
