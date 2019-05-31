INCDIR=../include

# customize build for arm toolchain
ifeq ($(TARGET),arm-none-eabi)
   CROSS_COMPILE=arm-none-eabi-
   CFLAGS+=-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections
   CFLAGS+= -DDISABLE_POLLING -DDISABLE_SOCKETS
else
endif

CC=$(CROSS_COMPILE)gcc
CFLAGS+= -I$(INCDIR) -O2 -std=c99 -Wall -Werror -pedantic
CFLAGS+= -U NDEBUG

AR=$(CROSS_COMPILE)ar



# -----------------------------------------------------------------
# build libfh_massage.a from sources in "src" and "src/test"
# -----------------------------------------------------------------
LIBOUT=libfh_message.a
SRC = $(wildcard *.c ./test/*.c)
OBJS = $(SRC:.c=.o)

%.o : %.c %.h
	$(CC) -c -I. $(CFLAGS) -o $@ $<


all: $(LIBOUT)

$(LIBOUT): $(OBJS)
	$(AR) rcs $@ $(OBJS)


# -----------------------------------------------------------------
# build test binaries found in tests/bin
#
# test bins sources are namded as:
# "test/bin/foo_main.c" -----> builds to "test/bin/foo"
# -----------------------------------------------------------------
_TEST_BINS = fh_selftest fh_transport_test
TEST_BINS = $(_TEST_BINS:%=test/bin/%)

tests: $(TEST_BINS)

$(TEST_BINS): %:%_main.c $(LIBOUT)
	$(CC) $(CFLAGS) -L. -o $@ $< $(LIBOUT:lib%.a=-l%)

self_test: ./test/bin/fh_selftest
	./test/bin/fh_selftest

.PHONY: clean

clean:
	rm -f *.o ./test/*.o $(LIBOUT)
	rm -f $(TEST_BINS)
