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
	LBCO	r0, C4, 4, 4
	CLR	r0, r0, 4
	SBCO	r0, C4, 4, 4

	// Put the GPIO1[28] pin into output mode.
	MOV	r1, GPIO1 | GPIO_OE
	LBBO	r2, r1, 0, 4	// r2 = current GPIO settings
	CLR	r2, PIN_BIT	// clear the bit => output mode
	SBBO	r2, r1, 0, 4	// write new GPIO settings

	MOV	r3, (1 << PIN_BIT)

MAIN_LOOP:
	// Read PRU DATA RAM.
	LBCO	r5, C24, 0, 8	// r5 = hi_delay, r6 = lo_delay

	// LO part of PWM.
	MOV	r4, GPIO1 | GPIO_SETDATAOUT
	SBBO	r3, r4, 0, 4	// send BIT28 to "set bit" address

DELAY1:
	SUB	r6, r6, 1	// bump delay counter
	QBNE	DELAY1, r6, 0 	// not done with delay yet?

	// HI part of PWM (the pulse).
	MOV	r4, GPIO1 | GPIO_CLEARDATAOUT
	SBBO	r3, r4, 0, 4	// send BIT28 to "clear bit" address

DELAY2:
	SUB	r5, r5, 1	// bump delay counter
	QBNE	DELAY2, r5, 0	// not done with delay yet?

	QBA	MAIN_LOOP	// infinite loop

