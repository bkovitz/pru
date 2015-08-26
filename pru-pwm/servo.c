/*
 * Pulse-width modulation on PRU to control SG90 microservo from BeagleBone
 *
 * Before running:
 *   Connect servo as follows:
 *     GND (brown wire) to BeagleBone P9.0 (GND)
 *     PWR (red wire)   to 5 VDC power source (not BeagleBone)
 *     SIGNAL (yellow wire) to BeagleBone P9.26 (GPIO1[28])
 *
 *   The enable_pru01 script must have been run. It's only needed once per
 *   reboot of the Beaglebone, to enable access to the PRU.
 *
 * To run:
 *   sudo ./servo
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
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU0 0
#define PRU1 1


typedef struct {
  unsigned int hi_delay;  // number of PRU loop iterations during pulse
  unsigned int lo_delay;  // number of PRU loop iterations between pulses
    // Each loop iteration takes 2 PRU clock cycles, or 10 ns.
} PRU_DELAYS;

PRU_DELAYS *pru_delays;     // Will point to PRU DATA RAM

char* pwm_binary_path = "pwm.bin";


#define CLOCKS_PER_uS 200 // clock cycles per microsecond (200 MHz PRU clock)
#define CLOCKS_PER_LOOP 2 // loop contains two instructions, one clock each

/*
 * We control the pulse width sent to the servo by writing to the PRU's
 * DATA RAM. The PRU reads it once every 20 ms to determine the width of
 * the next pulse to send out.
 *
 * pru_delays must point to the PRU DATA RAM before set_pulse_width() is
 * called.
 */

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

void sleep_tenths(unsigned int tenths) {  // tenths of a second
   static const struct timespec sleep_time = { 0, 100000000 };
   nanosleep(&sleep_time, NULL);
}

int main(int argc, char **argv) {
  int rtn;

  /* prussdrv_init() will segfault if called with EUID != 0 */
  if (geteuid()) {
    fprintf(stderr, "%s must be run as root to use prussdrv\n", argv[0]);
    return -1;
  }

  /* initialize PRU */
  if ((rtn = prussdrv_init()) != 0) {
    fprintf(stderr, "prussdrv_init() failed\n");
    return rtn;
  }

  /* Open the interrupt. This is needed to initialize the PRU even though
   * we're not using the interrupt. */
  if ((rtn = prussdrv_open(PRU_EVTOUT_0)) != 0) {
    fprintf(stderr, "prussdrv_open() failed\n");
    return rtn;
  }

  /* map PRU DATA RAM */
  prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void**)&pru_delays);

  /* Set up initial pulse width. This must be done before starting the PRU
   * so we don't signal the servo to turn further than it can go. */
  set_pulse_width(0);

  /* load and run the PRU program */
  if ((rtn = prussdrv_exec_program(PRU0, pwm_binary_path)) < 0) {
    fprintf(stderr, "prussdrv_exec_program() failed\n");
    return rtn;
  }

  /* Run the servo through its range. */
  int pulse_width;
  for (pulse_width = 0; pulse_width <= 1000; pulse_width += 50) {
    set_pulse_width(pulse_width);
    sleep_tenths(1);
  }

  /* clean up and exit */
  prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
  prussdrv_pru_disable(PRU0);
  prussdrv_exit();

  return 0;
}

