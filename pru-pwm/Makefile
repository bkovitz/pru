CFLAGS+=-Wall -Werror
LDLIBS+= -lpthread -lprussdrv

run: servo
	sudo ./servo

servo: servo.o pwm.bin
	$(CC) $(CFLAGS) -o servo servo.o $(LDLIBS)

pwm: pwm.bin main1
	sudo ./main1 pwm.bin
	
mem: mem.bin main1
	sudo ./main1 mem.bin


%.bin: %.p
	pasm -b $^

main: main.o

main1: main1.o

clean:
	rm -f main *.o *.bin

