
CC=gcc
FH_MESSAGE_DIR =../../../..
FH_EXAMPLE_DIR =../../..

INCDIRS=$(FH_MESSAGE_DIR)/include $(FH_EXAMPLE_DIR)/shared ../../bbb/include
LIBDIRS = $(FH_MESSAGE_DIR)/src $(FH_EXAMPLE_DIR)/shared
LIBS = fh_message fh_example pthread

CFLAGS=$(INCDIRS:%=-I%) -O2 -std=c99 -Wall -Werror -pedantic

BIN=sdaq


DEPS = $(FH_MESSAGE_DIR)/src/libfh_message.a $(FH_EXAMPLE_DIR)/shared/libfh_example.a
OBJS = sim_sdaq.o fh_peer.o state_ctrl.o fh_comp.o

%.o : %.c %.h $(DEPS)
	$(CC) -c -I. $(CFLAGS) $<


all: $(BIN)

sdaq: $(OBJS) sdaq.o
	$(CC) $(CFLAGS) $(LIBDIRS:%=-L%) -o $@  $(OBJS) $@.o $(LIBS:%=-l%)

# $(BIN): $(OBJS)
# 	$(CC) $(CFLAGS) -o $@   $(OBJS) $@.o

.PHONY: clean

clean:
	rm -f *.o $(BIN)
