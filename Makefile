CFLAGS := -std=gnu99 -Wall -Werror -O3 -g
C_SRCS := $(wildcard *.c)
C_OBJS := $(C_SRCS:.c=.o)
all:  tap2tap tap2tap-arm64 tap2tap-x86
#tap2tap-x86: tap2tap-x86
	x86_64-linux-gnu-gcc *.c -o tap2tap-x86 -static
#tap2tap-arm64: tap2tap-arm64
	aarch64-linux-gnu-gcc *.c -o tap2tap-arm64 -static
tap2tap: $(C_OBJS)
	$(CC) $(CFLAGS) $(C_OBJS) -o "$@" -static #-lsodium

%.o: %.c
	$(CC) $(CFLAGS) -c "$<"

.PHONY: test
test: tap2tap
	tox

.PHONY: clean
clean:
	rm -f *.o
	rm -f tap2tap tap2tap-arm64 tap2tap-x86 
