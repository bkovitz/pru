// Monitor GPIO1[16], which is pin P9.15 on the BeagleBone connectors.

.origin 0 		// offset of the start of the code in PRU memory
.entrypoint START	// program entry point, used by debugger only

#include "constants.h"

#define PIN_BIT 16

START:
	// Clear STANDBY_INIT in SYSCFG so PRU can access main memory.
	LBCO	r0, C4, 4, 4
	CLR	r0, r0, 4
	SBCO	r0, C4, 4, 4

	// Put the GPIO1[16] pin into input mode.
	MOV	r1, GPIO1 | GPIO_OE
	LBBO	r2, r1, 0, 4	// r2 = current GPIO settings
	SET	r2, PIN_BIT	// set the bit => input mode
	SBBO	r2, r1, 0, 4	// write new GPIO settings

	MOV	R1, GPIO1 | GPIO_DATAIN  // r1 -> GPIO input register
	MOV	R3, (1 << PIN_BIT)  // R3 = mask
SAMPLING_LOOP:
	LBBO	R2, R1, 0, 4	// read input register into r2
	AND	R2, R2, R3	// is our pin high?
	QBEQ	SAMPLING_LOOP, R2, 0  // no, keep sampling

        // tell host we're done, then halt
	MOV	R31.b0, PRU_R31_VEC_VALID | PRU_EVTOUT_0
	HALT


