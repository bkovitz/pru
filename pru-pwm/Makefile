CFLAGS+=-Wall -Werror
LDLIBS+= -lpthread -lprussdrv

run: all
	sudo ./main pwm.bin

all: pwm.bin main

clean:
	rm -f main *.o *.bin

pwm.bin: pwm.p
	pasm -b $^

main: main.o
