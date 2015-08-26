/*
 * Test of running two PRU programs simultaneously.
 *
 * Writes a number to each PRU's DATA RAM, and reads back what the program
 * wrote there when it raises PRU_EVTOUT_0 or PRU_EVTOUT_1.
 *
 * Before running:
 *   The enable_pru01 script must have been run. It's only needed once per
 *   reboot of the Beaglebone, to enable access to the PRU.
 *
 * Usage:
 *   ./sudo runpru pru0.bin pru1.bin
 *
 * By Ben Kovitz, August 2015, starting from example PRU code by Douglas
 * Henke available at:
 *   https://github.com/beagleboard/am335x_pru_package
 * Much gratitude!
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU0 0
#define PRU1 1


unsigned int *pru0_data_ram;
unsigned int *pru1_data_ram;

void usage() {
  fprintf(stderr, "usage: runpru pru0.bin [pru1.bin]\n\nEach .bin must be an assembled .p file.\n");
  exit(2);
}

int main(int argc, char **argv) {
  if (argc < 3)
    usage();

  if (geteuid()) {
    fprintf(stderr, "%s must be run as root\n", argv[0]);
    return 1;
  }

  if ((prussdrv_init()) != 0) {
    perror("prussdrv_init() failed");
    return 1;
  }

  if (prussdrv_open(PRU_EVTOUT_0) != 0) {
    perror("prussdrv_open(PRU_EVTOUT_0)");
    return 1;
  }

  if (prussdrv_open(PRU_EVTOUT_1) != 0) {
    perror("prussdrv_open(PRU_EVTOUT_1)");
    return 1;
  }

  prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void**)&pru0_data_ram);
  prussdrv_map_prumem(PRUSS0_PRU1_DATARAM, (void**)&pru1_data_ram);

  pru0_data_ram[0] = 0x1000;  // Initialize DATA RAM of each PRU unit
  pru1_data_ram[0] = 0x2000;

  static tpruss_intc_initdata intc = PRUSS_INTC_INITDATA;
  if (prussdrv_pruintc_init(&intc) != 0) {
    perror("prussdrv_pruintc_init()");
    return 1;
  }

  if (prussdrv_exec_program(PRU0, argv[1]) < 0) {
    perror(argv[1]);
  }

  if (prussdrv_exec_program(PRU1, argv[2]) < 0) {
    perror(argv[2]);
  }

  int pru_result0 = prussdrv_pru_wait_event(PRU_EVTOUT_0);
  prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

  printf("PRU0 program completed, event number %d\n", pru_result0);
  printf("Contents of PRU0 DATA RAM: %08x\n", pru0_data_ram[0]);

  int pru_result1 = prussdrv_pru_wait_event(PRU_EVTOUT_1);
  prussdrv_pru_clear_event(PRU_EVTOUT_1, PRU1_ARM_INTERRUPT);

  printf("PRU1 program completed, event number %d\n", pru_result1);
  printf("Contents of PRU1 DATA RAM: %08x\n", pru1_data_ram[0]);

  prussdrv_pru_disable(PRU0);
  prussdrv_pru_disable(PRU1);
  prussdrv_exit();

  return 0;
}

