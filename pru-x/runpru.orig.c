// Runs up to two programs on the PRU simultaneously.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRU_NUM 1 /* which of the two PRUs are we using? */


typedef struct {
  unsigned int hi_delay;  // number of PRU loop iterations during pulse
  unsigned int lo_delay;  // number of PRU loop iterations between pulses
    // Each loop iteration takes 2 PRU clock cycles, or 10 ns.
} PRU_DELAYS;

PRU_DELAYS *pruDataMem;              // Will point to PRU DATA RAM
//unsigned int *pruDataMem_int;  // Will point to PRU DATA RAM


/*** pru_setup() -- initialize PRU and interrupt handler

Initializes the PRU specified by PRU_NUM and sets up PRU_EVTOUT_1 handler.

The argument is a pointer to a nul-terminated string containing the path
to the file containing the PRU program binary.

Returns 0 on success, non-0 on error.
***/
static int pru_setup(const char * const path) {
   int rtn;
   tpruss_intc_initdata intc = PRUSS_INTC_INITDATA;

   if(!path) {
      fprintf(stderr, "pru_setup(): path is NULL\n");
      return -1;
   }

   /* initialize PRU */
   if((rtn = prussdrv_init()) != 0) {
      fprintf(stderr, "prussdrv_init() failed\n");
      return rtn;
   }

   /* open the interrupt */
   if((rtn = prussdrv_open(PRU_EVTOUT_1)) != 0) {
      fprintf(stderr, "prussdrv_open() failed\n");
      return rtn;
   }

   /* initialize interrupt */
   if((rtn = prussdrv_pruintc_init(&intc)) != 0) {
      fprintf(stderr, "prussdrv_pruintc_init() failed\n");
      return rtn;
   }

   /* map PRU DATA RAM */
   prussdrv_map_prumem(PRUSS0_PRU1_DATARAM, (void**)&pruDataMem);
   pruDataMem->hi_delay = 1000;
   pruDataMem->lo_delay = 19000;

   /* load and run the PRU program */
   if((rtn = prussdrv_exec_program(PRU_NUM, path)) < 0) {
      fprintf(stderr, "prussdrv_exec_program() failed\n");
      return rtn;
   }

   return rtn;
}

/*** pru_cleanup() -- halt PRU and release driver

Performs all necessary de-initialization tasks for the prussdrv library.

Returns 0 on success, non-0 on error.
***/
static int pru_cleanup(void) {
   int rtn = 0;

   /* clear the event (if asserted) */
   if(prussdrv_pru_clear_event(PRU_EVTOUT_1, PRU1_ARM_INTERRUPT)) {
      fprintf(stderr, "prussdrv_pru_clear_event() failed\n");
      rtn = -1;
   }

   /* halt and disable the PRU (if running) */
   if((rtn = prussdrv_pru_disable(PRU_NUM)) != 0) {
      fprintf(stderr, "prussdrv_pru_disable() failed\n");
      rtn = -1;
   }

   /* release the PRU clocks and disable prussdrv module */
   if((rtn = prussdrv_exit()) != 0) {
      fprintf(stderr, "prussdrv_exit() failed\n");
      rtn = -1;
   }

   return rtn;
}

int old_main(int argc, char **argv) {
   int rtn;

   /* prussdrv_init() will segfault if called with EUID != 0 */ 
   if(geteuid()) {
      fprintf(stderr, "%s must be run as root to use prussdrv\n", argv[0]);
      return -1;
   }

   /* initialize the library, PRU and interrupt; launch our PRU program */
   if(pru_setup("./pwm.bin")) {
      pru_cleanup();
      return -1;
   }

   /* wait for PRU to assert the interrupt to indicate completion */
   printf("waiting for interrupt from PRU1...\n");

   /* The prussdrv_pru_wait_event() function returns the number of times
      the event has taken place, as an unsigned int. There is no out-of-
      band value to indicate error (and it can wrap around to 0 if you
      run the program just a whole lot of times). */
   rtn = prussdrv_pru_wait_event(PRU_EVTOUT_1);

   printf("PRU program completed, event number %d\n", rtn);

   /* clear the event, disable the PRU and let the library clean up */
   return pru_cleanup();
}

#define CLOCKS_PER_uS 200 // clock cycles per microsecond (200 MHz PRU clock)
#define CLOCKS_PER_LOOP 2 // loop contains two instructions, one clock each

void set_pulse_width(unsigned int pulse_width) {  // 0..1000 uS
   // Actual pulse width is 1 ms + pulse_width. Total PWM cycle time
   // is always 20 ms.
  pruDataMem->hi_delay =
      (1000 + pulse_width) * CLOCKS_PER_uS / CLOCKS_PER_LOOP;
  pruDataMem->lo_delay =
      (19000 - pulse_width) * CLOCKS_PER_uS / CLOCKS_PER_LOOP;
  printf("hi_delay=%d lo_delay=%d\n",
      pruDataMem->hi_delay, pruDataMem->lo_delay);
}

void sleep_tenths(unsigned int tenths) {  // tenths of a second
   static const struct timespec sleep_time = { 0, 100000000 };
   nanosleep(&sleep_time, NULL);
}

void usage() {
  fprintf(stderr, "usage: pru file.bin\n\nfile.bin must be an assembled .p file.\n");
  exit(2);
}

int main(int argc, char **argv) {
   int rtn;

   if (argc != 2)
    usage();

   /* prussdrv_init() will segfault if called with EUID != 0 */ 
   if(geteuid()) {
      fprintf(stderr, "%s must be run as root to use prussdrv\n", argv[0]);
      return -1;
   }

   /* initialize the library, PRU and interrupt; launch our PRU program */
   if(pru_setup(argv[1])) {
      pru_cleanup();
      return -1;
   }

   int pulse_width;
   for (pulse_width = 0; pulse_width <= 1000; pulse_width += 50) {
     set_pulse_width(pulse_width);
     sleep_tenths(1);
   }

   /* disable the PRU */
   if((rtn = prussdrv_pru_disable(PRU_NUM)) != 0) {
      fprintf(stderr, "prussdrv_pru_disable() failed\n");
      rtn = -1;
   }

//   /* wait for PRU to assert the interrupt to indicate completion */
//   printf("waiting for interrupt from PRU1...\n");
//
//   /* The prussdrv_pru_wait_event() function returns the number of times
//      the event has taken place, as an unsigned int. There is no out-of-
//      band value to indicate error (and it can wrap around to 0 if you
//      run the program just a whole lot of times). */
//   rtn = prussdrv_pru_wait_event(PRU_EVTOUT_1);
//
//   printf("PRU program completed, event number %d\n", rtn);

   //int *pruDataMem_int = (int*)pruDataMem;

   //printf("Contents of PRU DATA RAM: %08x\n", pruDataMem[0]);
   printf("Contents of PRU DATA RAM: %08x\n", pruDataMem->hi_delay);

   /* clear the event, disable the PRU and let the library clean up */
   return pru_cleanup();
}


