CFLAGS+=-Wall -Werror
LDLIBS+= -lpthread -lprussdrv

dualtest: runtwo pru0.bin pru1.bin
	sudo ./runtwo pru0.bin pru1.bin

runservo: servo
	sudo ./servo

servo: servo.o pwm.bin
	$(CC) $(CFLAGS) -o servo servo.o $(LDLIBS)

#pwm: pwm.bin runpru
#	sudo ./runpru pwm.bin
#	
#mem: mem.bin runpru
#	sudo ./runpru mem.bin

%.bin: %.p
	pasm -b $^

clean:
	rm -f runpru servo *.o *.bin

