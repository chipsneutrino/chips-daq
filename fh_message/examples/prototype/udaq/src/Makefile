
CC=gcc
FH_MESSAGE_DIR = ../../../..
FH_EXAMPLE_DIR = ../../..

INCDIRS = $(FH_MESSAGE_DIR)/include $(FH_EXAMPLE_DIR)/shared ../include
LIBDIRS = $(FH_MESSAGE_DIR)/src $(FH_EXAMPLE_DIR)/shared
LIBS = fh_message fh_example pthread

CFLAGS = $(INCDIRS:%=-I%) -O2 -std=c99 -Wall -Werror -pedantic

BIN = udaq


DEPS = $(FH_MESSAGE_DIR)/src/libfh_message.a $(FH_EXAMPLE_DIR)/shared/libfh_example.a $(wildcard ../include/*.h)
OBJS = msg_service.o data_access_service.o sim_udaq.o

%.o : %.c %.h $(DEPS)
	$(CC) -c -I. $(CFLAGS) $<


all: $(BIN)

udaq: $(OBJS) udaq.o
	$(CC) $(CFLAGS) $(LIBDIRS:%=-L%) -o $@  $(OBJS) $@.o $(LIBS:%=-l%)


.PHONY: clean

clean:
	rm -f *.o $(BIN)
