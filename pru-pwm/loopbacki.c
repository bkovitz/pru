/*
 * BeagleBone loopback PWM test: measure pulse widths by logging timestamps
 * on the PRU. Just like loopback2, but ARM is notified of new pulses from
 * the PRU by interrupt rather than by examining a log.
 *
 * Generates PWM wave with varying pulse widths at GPIO1[28] (P9.12) and
 * samples at GPIO1[16] (P9.15).
 *
 * RESULT:
 *   ???
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
#include <time.h>
#include <errno.h>
#include <string.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include "constants.h"

#define PRU0 0    // Generate pulses on PRU0
#define PRU1 1    // Sample pulses on PRU1

#define NUM_SAMPLES 200

extern unsigned char pwm_bin[];   // generated by xxd from pwm.p
extern unsigned int pwm_bin_len;
extern unsigned char logpulses_bin[];
                                  // generated by xxd from logpulses_bin.p
extern unsigned int logpulses_bin_len;

#define CLOCKS_PER_uS 200 // clock cycles per microsecond (200 MHz PRU clock)
#define CLOCKS_PER_LOOP 2 // loop contains two instructions, one clock each

typedef struct {
  unsigned int hi_delay;  // number of PRU loop iterations during pulse
  unsigned int lo_delay;  // number of PRU loop iterations between pulses
    // Each loop iteration takes 2 PRU clock cycles, or 10 ns.
} PRU_DELAYS;

typedef struct {
  signed int pulse_start;
  signed int pulse_end;
} SAMPLE;

typedef struct {
  unsigned int gpio_base;   // base address of GPIO register
  unsigned char pin_bit;    // which pin to read (0..31)
  unsigned char pru_evtout; // which PRU_EVTOUT to signal on exit
  unsigned char num_samples; // number of pulses to read before exiting
  SAMPLE samples[NUM_SAMPLES]; // filled in by PRU
} PRU_LOG;

PRU_DELAYS *pru_delays;     // Will point to PRU0 DATA RAM
PRU_LOG *pru1_data_ram;

void set_pulse_width(unsigned int pulse_width) {  // 0..1000 uS
   // Actual pulse width is 1 ms + pulse_width. Total PWM cycle time
   // is always 20 ms.
  pru_delays->hi_delay =
      (1000 + pulse_width) * CLOCKS_PER_uS / CLOCKS_PER_LOOP;
  pru_delays->lo_delay =
      (19000 - pulse_width) * CLOCKS_PER_uS / CLOCKS_PER_LOOP;
  printf("hi_delay=%d lo_delay=%d\n",
      pru_delays->hi_delay, pru_delays->lo_delay);
}

void sleep_millis(unsigned int millis) {  // milliseconds
   const struct timespec sleep_time = { 0, millis * 1000000 };
   nanosleep(&sleep_time, NULL);
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

  if (prussdrv_open(PRU_EVTOUT_0) != 0) {
    perror("prussdrv_open(PRU_EVTOUT_0)");
    return 1;
  }

  if (prussdrv_open(PRU_EVTOUT_1) != 0) {
    perror("prussdrv_open(PRU_EVTOUT_1)");
    return 1;
  }

  /* map PRU DATA RAM */
  prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void**)&pru_delays);
  prussdrv_map_prumem(PRUSS0_PRU1_DATARAM, (void**)&pru1_data_ram);

  memset((void *)pru1_data_ram, 0x99, sizeof(PRU_LOG));

  set_pulse_width(0);

  static tpruss_intc_initdata intc = PRUSS_INTC_INITDATA;
  if (prussdrv_pruintc_init(&intc) != 0) {
    perror("prussdrv_pruintc_init()");
    return 1;
  }

  if (prussdrv_pru_write_memory(
        PRUSS0_PRU0_IRAM, 0, (unsigned int *)pwm_bin, pwm_bin_len
     ) != pwm_bin_len / 4) {
    perror("prussdrv_pru_write_memory(PRU0)");
    return 1;
  }

  if (prussdrv_pru_write_memory(
        PRUSS0_PRU1_IRAM, 0, (unsigned int *)logpulses_bin,
        logpulses_bin_len
     ) != logpulses_bin_len / 4) {
    perror("prussdrv_pru_write_memory(PRU1)");
    return 1;
  }

  pru1_data_ram->gpio_base = GPIO1;
  pru1_data_ram->pin_bit = 16;
  pru1_data_ram->pru_evtout = PRU_EVTOUT_1_CODE;
  pru1_data_ram->num_samples = NUM_SAMPLES;

  if (prussdrv_pru_enable(PRU0) != 0) {
    perror("prussdrv_pru_enable(0)");
    return 1;
  }

  if (prussdrv_pru_enable(PRU1) != 0) {
    perror("prussdrv_pru_enable(1)");
    return 1;
  }

  for (int i = 0; i < NUM_SAMPLES + 20; i++) {
    set_pulse_width(i);
    sleep_millis(20);
  }
  set_pulse_width(0);

  //int pru_result0 = prussdrv_pru_wait_event(PRU_EVTOUT_0);
  //prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

  //printf("PRU0 program completed, event number %d\n", pru_result0);

  int pru_result1 = prussdrv_pru_wait_event(PRU_EVTOUT_1);
  prussdrv_pru_clear_event(PRU_EVTOUT_1, PRU1_ARM_INTERRUPT);

  printf("PRU1 program completed, event number %d\n", pru_result1);

  for (int i = 0; i < NUM_SAMPLES; i++) {
    signed int start = pru1_data_ram->samples[i].pulse_start;
    signed int end =   pru1_data_ram->samples[i].pulse_end;
    

    printf("0x%08x 0x%08x %d %9.3f\n", start, end, end - start,
      1000000.0 * (end - start) / 24000000.0);
  }

  prussdrv_pru_disable(PRU0);
  prussdrv_pru_disable(PRU1);
  prussdrv_exit();

  return 0;
}

