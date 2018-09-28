
CC=gcc
FH_MESSAGE_DIR =../../../..
FH_EXAMPLE_DIR =../../..

INCDIRS=$(FH_MESSAGE_DIR)/include $(FH_EXAMPLE_DIR)/shared ../include ../../udaq/include
LIBDIRS = $(FH_MESSAGE_DIR)/src $(FH_EXAMPLE_DIR)/shared
LIBS = fh_message fh_example pthread

#CFLAGS=$(INCDIRS:%=-I%) -O2 -std=c99 -Wall -Werror -pedantic
	
# _GNUSOURCE or _POSIX_C_SOURCE required for getopts funtionality
#CFLAGS=$(INCDIRS:%=-I%) -O2 -std=c99 -Wall -Werror -pedantic -D_POSIX_C_SOURCE=199309L
CFLAGS=$(INCDIRS:%=-I%) -O2 -std=c99 -Wall -Werror -pedantic -D_GNU_SOURCE

BIN=bbb


DEPS = $(FH_MESSAGE_DIR)/src/libfh_message.a $(FH_EXAMPLE_DIR)/shared/libfh_example.a msg_service_api.h exp_ctrl_service_api.h data_access_service_api.h
OBJS = msg_service.o data_access_service.o sim_bbb.o pagebuf.o udaq_peer.o state_ctrl.o udaq_comp.o udaq_comp_group.o

%.o : %.c %.h $(DEPS)
	$(CC) -c -I. $(CFLAGS) $<


all: $(BIN)

bbb: $(OBJS) bbb.o
	$(CC) $(CFLAGS) $(LIBDIRS:%=-L%) -o $@  $(OBJS) $@.o $(LIBS:%=-l%)


# $(BIN): $(OBJS)
# 	$(CC) $(CFLAGS) -o $@   $(OBJS) $@.o

.PHONY: clean

clean:
	rm -f *.o $(BIN)
