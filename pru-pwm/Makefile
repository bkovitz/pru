CFLAGS+=-Wall -Werror -O3 -std=gnu99
LDLIBS+= -lpthread -lprussdrv

run: sample
	sudo ./sample

runservo: servo
	sudo ./servo

servo: servo.o pwm.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

sample: sample.o prusample.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

dualtest: runtwo pru0.bin pru1.bin
	sudo ./runtwo pru0.bin pru1.bin

#pwm: pwm.bin runpru
#	sudo ./runpru pwm.bin
#	
#mem: mem.bin runpru
#	sudo ./runpru mem.bin

%.bin: %.p
	pasm -b $^

%.c: %.bin
	xxd -i $^ > $@

%.o: %.p

clean:
	rm -f runpru servo gpiodirect *.o *.bin

