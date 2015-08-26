CFLAGS+=-Wall -Werror
LDLIBS+= -lpthread -lprussdrv

pwm: pwm.bin main
	sudo ./main pwm.bin
	
mem: mem.bin main
	sudo ./main mem.bin


pwm.bin: pwm.p
	pasm -b $^

mem.bin: mem.p
	pasm -b $^

main: main.o

clean:
	rm -f main *.o *.bin

