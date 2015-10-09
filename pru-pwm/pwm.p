// Simple pulse-width modulation (PWM) program for PRU
//
// Sends pulses of specified duration to pin P9.12 on the BeagleBone.
//
// By Ben Kovitz, August 2015.

.origin 0 // offset of the start of the code in PRU memory
.entrypoint START // program entry point, used by debugger only

#include "constants.h"

// We'll toggle GPIO1[28], which is P9.12 on the BeagleBone connectors.
#define PIN_BIT 28

START:
	// Clear STANDBY_INIT in SYSCFG so PRU can access main memory.
	lbco	r0, C4, SYSCFG, 4
	clr	r0, r0, 4	// STANDBY_INIT is bit 4
	sbco	r0, C4, SYSCFG, 4

	// Make sure that C24 -> start of PRU DATA RAM
	mov	r0, 0x0
	mov	r1, PRU_ICSS_PRU0_CTRL	// hard-coded PRU unit??
	sbbo	r0, r1, CTBIR0, 1

	// Put the GPIO1[28] pin into output mode.
	mov	r1, GPIO1 | GPIO_OE
	lbbo	r2, r1, 0, 4	// r2 = current GPIO settings
	clr	r2, PIN_BIT	// clear the bit => output mode
	sbbo	r2, r1, 0, 4	// write new GPIO settings

	mov	r3, (1 << PIN_BIT)

MAIN_LOOP:
	// Read PRU DATA RAM.
	lbco	r5, C24, 0, 8	// r5 = hi_delay, r6 = lo_delay

	// LO part of PWM.
	mov	r4, GPIO1 | GPIO_SETDATAOUT
	sbbo	r3, r4, 0, 4	// send BIT28 to "set bit" address

DELAY1:
	sub	r6, r6, 1	// bump delay counter
	qbne	DELAY1, r6, 0 	// not done with delay yet?

	// HI part of PWM (the pulse).
	mov	r4, GPIO1 | GPIO_CLEARDATAOUT
	sbbo	r3, r4, 0, 4	// send BIT28 to "clear bit" address

DELAY2:
	sub	r5, r5, 1	// bump delay counter
	qbne	DELAY2, r5, 0	// not done with delay yet?

	qba	MAIN_LOOP	// infinite loop

