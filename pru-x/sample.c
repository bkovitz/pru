/*
 * Test of sampling GPIO pin on PRU.
 *
 * PRU0 keeps sampling until pin goes high.
 *
 * Before running:
 *   The enable_pru01 script must have been run. It's only needed once per
 *   reboot of the Beaglebone, to enable access to the PRU.
 *
 * Usage:
 *   ./sudo sample
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


extern unsigned char prusample_bin[];  // generated by xxd from prusample.p
extern unsigned int prusample_bin_len;

int main(int argc, char **argv) {
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

  static tpruss_intc_initdata intc = PRUSS_INTC_INITDATA;
  if (prussdrv_pruintc_init(&intc) != 0) {
    perror("prussdrv_pruintc_init()");
    return 1;
  }

  if (prussdrv_pru_write_memory(
        PRUSS0_PRU0_IRAM, 0, (unsigned int *)prusample_bin, prusample_bin_len
     ) != prusample_bin_len / 4) {
    perror("prussdrv_pru_write_memory()");
    return 1;
  }

  if (prussdrv_pru_enable(PRU0) != 0) {
    perror("prussdrv_pru_enable()");
    return 1;
  }

  int pru_result0 = prussdrv_pru_wait_event(PRU_EVTOUT_0);
  prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

  printf("PRU0 program completed, event number %d\n", pru_result0);

  prussdrv_pru_disable(PRU0);
  prussdrv_exit();

  return 0;
}

