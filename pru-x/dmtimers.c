/*
 * Examine DMTIMER settings on BeagleBone.
 *
 * RESULT:
 *   The BeagleBone already has DMTIMER2 running at top speed, with an
 *   interrupt to turn it over on overflow. In 1 sec, it counts
 *   approximately 24,000,000 ticks.
 *
 *   DMTIMER0 and DMTIMER2..7 are all set to take their clock input from
 *   M_OSC, which is set at 24 Mhz as shown by SYSBOOT[15:14].
 *
 *   DMTIMER0 and DMTIMER3..7 are unused. I did not look at DMTIMER1,
 *   since it's a millisecond timer, and works differently.
 *
 * Before running:
 *   The enable_pru01 script must have been run. It's only needed once per
 *   reboot of the Beaglebone, to enable access to the PRU.
 *
 * Usage:
 *   ./sudo loopback
 *
 * By Ben Kovitz, August 2015, starting from example PRU code by Douglas
 * Henke available at:
 *   https://github.com/beagleboard/am335x_pru_package
 * Much gratitude!
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU0 0    // Generate pulses on PRU0
#define PRU1 1    // Sample pulses on PRU1

#define NUM_SAMPLES 100

extern unsigned char timerblock_bin[];   // generated by xxd from timerblock.p
extern unsigned int timerblock_bin_len;

typedef struct {
  unsigned int tclr;  // timer control register
  unsigned int tcrr;  // timer counter register
  unsigned int tldr;  // timer load register
  unsigned int clkctrl;  // CM_PER_TIMERx_CLKCTRL register from CM_PER
  unsigned int clksel;   // CLKSEL_TIMERx_CLK register from CM_DPLL
} DMTIMER_INFO;

typedef struct {
  unsigned int control_status; // indicates crystal frequency in bits [23:22]
  unsigned int l4s_clkstctrl; // CM_PER_L4LS_CLKSTCTRL
  DMTIMER_INFO dmtimer_infos[7];
} PRU_RAM;

PRU_RAM *pru1_data_ram;

void sleep_millis(unsigned int millis) {  // milliseconds
   const struct timespec sleep_time = { 0, millis * 1000000 };
   nanosleep(&sleep_time, NULL);
}

void print_dmtimer_info(const char *name, DMTIMER_INFO *dmtimer_info) {
  printf("%s\n", name);
  printf("  TCLR = 0x%08x\n", dmtimer_info-> tclr);
  printf("  TCRR = 0x%08x\n", dmtimer_info-> tcrr);
  printf("  TLDR = 0x%08x\n", dmtimer_info-> tldr);
  printf("  CLKCTRL = 0x%08x\n", dmtimer_info->clkctrl);
  printf("  CLKSEL = 0x%08x\n", dmtimer_info->clksel);
}

void print_pru_ram() {
  printf("CONTROL_STATUS = 0x%08x\n", pru1_data_ram->control_status);
  printf("CM_PER_L4LS_CLKSTCTRL = 0x%08x\n", pru1_data_ram->l4s_clkstctrl);
  print_dmtimer_info("DMTIMER0", &pru1_data_ram->dmtimer_infos[0]);
  print_dmtimer_info("DMTIMER2", &pru1_data_ram->dmtimer_infos[1]);
  print_dmtimer_info("DMTIMER3", &pru1_data_ram->dmtimer_infos[2]);
  print_dmtimer_info("DMTIMER4", &pru1_data_ram->dmtimer_infos[3]);
  print_dmtimer_info("DMTIMER5", &pru1_data_ram->dmtimer_infos[4]);
  print_dmtimer_info("DMTIMER6", &pru1_data_ram->dmtimer_infos[5]);
  print_dmtimer_info("DMTIMER7", &pru1_data_ram->dmtimer_infos[6]);
}

void run_pru(unsigned pru, unsigned evtout) {
  if (prussdrv_pru_enable(pru) != 0) {
    perror("prussdrv_pru_enable()");
    exit(1);
  }

  int pru_result1 = prussdrv_pru_wait_event(evtout);
  prussdrv_pru_clear_event(evtout, PRU1_ARM_INTERRUPT);

  printf("PRU%d program completed, event number %d\n", pru, pru_result1);

  print_pru_ram();
}

int main(int argc, char **argv) {
  if (geteuid()) {
    fprintf(stderr, "%s must be run as root\n", argv[0]);
    return 1;
  }

  if ((prussdrv_init()) != 0) {
    perror("prussdrv_init() failed");
    return 1;
  }

  if (prussdrv_open(PRU_EVTOUT_1) != 0) {
    perror("prussdrv_open(PRU_EVTOUT_1)");
    return 1;
  }

  /* map PRU DATA RAM */
  prussdrv_map_prumem(PRUSS0_PRU1_DATARAM, (void **)&pru1_data_ram);

  memset((void *)pru1_data_ram, 0x99, sizeof(PRU_RAM));

  static tpruss_intc_initdata intc = PRUSS_INTC_INITDATA;
  if (prussdrv_pruintc_init(&intc) != 0) {
    perror("prussdrv_pruintc_init()");
    return 1;
  }

  if (prussdrv_pru_write_memory(
        PRUSS0_PRU1_IRAM, 0, (unsigned int *)timerblock_bin, timerblock_bin_len
     ) != timerblock_bin_len / 4) {
    perror("prussdrv_pru_write_memory(PRU1)");
    return 1;
  }

  run_pru(PRU1, PRU_EVTOUT_1);
  int first_count = pru1_data_ram->dmtimer_infos[1].tcrr;
  printf("\n");
  sleep(1);
  run_pru(PRU1, PRU_EVTOUT_1);
  printf("\n");
  int second_count = pru1_data_ram->dmtimer_infos[1].tcrr;
  printf("DMTIMER2.TCRR difference: %d\n", second_count - first_count);

  prussdrv_pru_disable(PRU1);
  prussdrv_exit();

  return 0;
}

