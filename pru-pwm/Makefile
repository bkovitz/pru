CFLAGS+=-Wall -Werror -O3 -std=gnu99
LDLIBS+= -lpthread -lprussdrv

runloopback2: loopback2
	sudo ./loopback2

runtcapture: tcapture
	sudo ./tcapture

rundmtimers: dmtimers
	sudo ./dmtimers

runloopback: loopback
	sudo ./loopback

all: servo sample loopback runtwo dmtimers gpiodirect tcapture

runservo: servo
	sudo ./servo

servo: servo.o pwm.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

sample: sample.o prusample.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

loopback: loopback.o pwm.o samplecount.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

loopback2: loopback2.o pwm.o logpulses.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

dmtimers: dmtimers.o timerblock.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

dualtest: runtwo pru0.bin pru1.bin
	sudo ./runtwo pru0.bin pru1.bin

tcapture: tcapture.o timercapt.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.bin: %.p
	pasm -b $^

%.c: %.bin
	xxd -i $^ > $@

# Disable built-in Pascal compilation rule
%.o: %.p

clean:
	rm -f runtwo servo loopback dmtimers gpiodirect sample tcapture *.o *.bin

