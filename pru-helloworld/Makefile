CFLAGS+=-Wall -Werror
LDLIBS+= -lpthread -lprussdrv

all: example.bin example

clean:
	rm -f example *.o *.bin

example.bin: example.p
	pasm -b $^

example: example.o
