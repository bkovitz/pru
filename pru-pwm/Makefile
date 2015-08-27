CFLAGS+=-Wall -Werror -O3 -std=gnu99
LDLIBS+= -lpthread -lprussdrv

run: loopback
	sudo ./loopback

runservo: servo
	sudo ./servo

servo: servo.o pwm.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

sample: sample.o prusample.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

loopback: loopback.o pwm.o samplecount.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

dualtest: runtwo pru0.bin pru1.bin
	sudo ./runtwo pru0.bin pru1.bin

%.bin: %.p
	pasm -b $^

%.c: %.bin
	xxd -i $^ > $@

# Disable built-in Pascal compilation rule
%.o: %.p

clean:
	rm -f runpru servo gpiodirect *.o *.bin

