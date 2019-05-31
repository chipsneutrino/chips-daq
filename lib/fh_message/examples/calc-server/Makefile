
CC=gcc
FH_MESSAGE_DIR =../..
FH_EXAMPLE_DIR =..

INCDIRS=$(FH_MESSAGE_DIR)/include $(FH_EXAMPLE_DIR)/shared
LIBDIRS = $(FH_MESSAGE_DIR)/src $(FH_EXAMPLE_DIR)/shared
LIBS = fh_message fh_example pthread

CFLAGS=$(INCDIRS:%=-I%) -O2 -std=c99 -Wall -Werror -pedantic

BIN=server client


DEPS = $(FH_MESSAGE_DIR)/src/libfh_message.a $(FH_EXAMPLE_DIR)/shared/libfh_example.a server_api.h calc_api.h
OBJS = calc_service.o

%.o : %.c %.h $(DEPS)
	$(CC) -c -I. $(CFLAGS) $<


all: $(BIN)

server: $(OBJS) server.o $(DEPS)
	$(CC) $(CFLAGS) $(LIBDIRS:%=-L%) -o $@  $(OBJS) $@.o $(LIBS:%=-l%)
client: $(OBJS) client.o
	$(CC) $(CFLAGS) $(LIBDIRS:%=-L%) -o $@  $(OBJS) $@.o $(LIBS:%=-l%)

.PHONY: clean

clean:
	rm -f *.o $(BIN)
