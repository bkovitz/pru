#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define GPIO1 0x4804c000
#define GPIO_OE 0x134   // output enable
#define GPIO_SETDATAOUT 0x190
#define GPIO_CLEARDATAOUT 0x194
#define GPIO_SIZE 0xfff

#define gpio1(offset) (*(gpio + (offset) / 4))

#define PIN28 (1 << 28)

int mem_fd;
volatile unsigned int *gpio;

int main(int argc, char *argv[]) {
  printf("First.\n");

  if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0) {
    printf("can't open /dev/mem\n");
    exit(1);
  }

  char *gpio_map = (char *)mmap(
    0,
    GPIO_SIZE,
    PROT_READ|PROT_WRITE,
    MAP_SHARED,
    mem_fd,
    GPIO1
  );

  printf("gpio_map = %p\n", gpio_map);

  gpio = (volatile unsigned int *)gpio_map;

  int oe = gpio1(GPIO_OE);

  printf("OE = 0x%08x %d\n", oe, oe & PIN28);

  for (int i = 0; i < 10000000; i++) {
    gpio1(GPIO_SETDATAOUT) = PIN28;
    gpio1(GPIO_CLEARDATAOUT) = PIN28;
  }


  //printf("%08x\n", gpiooe);
  //printf("%08x\n", *(volatile int *)(GPIO1 | GPIO_OE));
//  *(volatile int *)(GPIO1 | GPIO_OE) &= ~(1 << 28);
//  printf("%08x\n", *(volatile int *)(GPIO1 | GPIO_OE));
//  *(volatile int *)(GPIO1 | GPIO_CLEARDATAOUT) = 1 << 28;
  printf("Done.\n");
  return 0;
}
