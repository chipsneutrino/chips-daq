
CC=gcc
FH_EXAMPLE_DIR =..

INCDIRS = $(FH_EXAMPLE_DIR)/shared
LIBDIRS = $(FH_EXAMPLE_DIR)/shared
LIBS = fh_example pthread

CFLAGS=$(INCDIRS:%=-I%) -O2 -std=c99 -Wall -Werror -pedantic

BIN=main


DEPS = $(FH_EXAMPLE_DIR)/shared/libfh_example.a
OBJS =

%.o : %.c %.h $(DEPS)
	$(CC) -c -I. $(CFLAGS) $<


all: $(BIN)

main: $(OBJS) main.o $(DEPS)
	$(CC) $(CFLAGS) $(LIBDIRS:%=-L%) -o $@  $(OBJS) $@.o $(LIBS:%=-l%)

.PHONY: clean

clean:
	rm -f *.o $(BIN)
