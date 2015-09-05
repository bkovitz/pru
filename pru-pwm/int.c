/*
 * Simple test to catch interrupts from PRU.
 *
 * RESULTS
 *   Found an error in the API documentation regarding
 *   prussdrv_pru_clear_event(). See pruevtout0_thread() below.
 *
 *   It appears to be much easier to make a pthread that waits for the PRU
 *   interrupt than to make an interrupt handler that works under Linux.
 *   Hopefully this will not chew up the ARM CPU. prussdrv_pru_wait_event()
 *   appears to be implemented as a blocking read.
 *
 *   If the PRU generates interrupts as fast as it can, the ARM can't keep
 *   up. But this is OK: the missed interrupts don't block the PRU and
 *   they don't block calls to prussdrv_pru_wait_event() or
 *   prussdrv_pru_clear_event().
 *
 * Based on Skeleton Application Code at:
 * http://processors.wiki.ti.com/index.php/PRU_Linux_Application_Loader_API_Guide
 *
 * By Ben Kovitz, September 2015.
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

extern unsigned char intp_bin[];   // generated by xxd from intp.p
extern unsigned char intp_bin_len;

int *pru0_data_ram;

void enable_pru_interrupts() {
  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
  prussdrv_pruintc_init(&pruss_intc_initdata);
}

void *pruevtout0_thread(void *arg) {
  printf("pruevtout0_thread started.\n");
  printf("count = %d\n", *pru0_data_ram);
  do {
    prussdrv_pru_wait_event(PRU_EVTOUT_0);
    printf("count = %d\n", *pru0_data_ram);
    // The Linux API documentation is wrong regarding the following function:
    // the documentation has the arguments reversed. If you get this wrong,
    // the above prussdrv_pru_wait_event() will wait forever even if
    // an interrupt is pending.
    prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
  } while (1);
}

int main(void) {

  prussdrv_init();
  prussdrv_open(PRU_EVTOUT_0);
  prussdrv_pru_reset(PRU0);

  enable_pru_interrupts();
  
  prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void**)&pru0_data_ram);
  *pru0_data_ram = 99;
  prussdrv_pru_write_memory(
    PRUSS0_PRU0_IRAM, 0, (unsigned int*)intp_bin, intp_bin_len
  );

// Contra the ARM PRU Linux API documentation, this function does not exist!
//  prussdrv_start_irqthread(
//    PRU_EVTOUT_0, sched_get_priority_max(SCHED_FIFO) - 2, pruevtout0_thread
//  );

  pthread_t irqthread;
  int iret = pthread_create(&irqthread, NULL, pruevtout0_thread, NULL);
  printf("iret = %d\n", iret);
  sleep(1);

  printf("starting PRU...\n");
  prussdrv_pru_enable(PRU0);

  sleep(3);
  printf("after sleep\n");
  printf("count = %d\n", *pru0_data_ram);
  prussdrv_pru_disable(PRU0);
  printf("PRU disabled\n");
  printf("count = %d\n", *pru0_data_ram);
}
