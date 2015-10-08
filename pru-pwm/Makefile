ALL=thrloopback seegps

CFLAGS+=-Wall -Werror -O3 -std=gnu99 -lm -lgps
LDLIBS+= -lpthread -lprussdrv

all: $(ALL)

runthrloopback: thrloopback
	sudo ./thrloopback

thrloopback: thrloopback.o pwm.o measurep.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.bin: %.p
	pasm -b $^

%.c: %.bin
	xxd -i $^ > $@

# Disable built-in Pascal compilation rule
%.o: %.p

clean:
	rm -f $(ALL) *.o *.bin

