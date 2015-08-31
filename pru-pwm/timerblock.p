// Look at DMTIMER registers and copy them to PRU DATA RAM.
//
// Raises interrupt PRU_EVTOUT_1 on completion.

.origin 0 		// offset of the start of the code in PRU memory
.entrypoint start	// program entry point, used by debugger only

#include "constants.h"

start:
	// Clear STANDBY_INIT in SYSCFG so PRU can access main memory.
	lbco	r0, c4, 4, 4
	clr	r0, r0, 4
	sbco	r0, c4, 4, 4

	mov	r0, DMTIMER0	// r0 -> DMTIMER0 block
	mov	r1, 0		// r1 = offset into pru1_data_mem of where
				//   to store data from DMTIMER block
	lbbo	r2, r0, TCLR, 12	// r2..r4 = TCLR, TCRR, TLDR
	sbco	r2, c24, r1.b0, 12	// write to pru1_data_mem

	mov	r0, DMTIMER2		// r0 -> DMTIMER2 block
	add	r1, r1, 12		// r1 = offset into pru1_data_mem
	lbbo	r2, r0, TCLR, 12	// r2..r4 = TCLR, TCRR, TLDR
	sbco	r2, c24, r1, 12		// write to pru1_data_mem

	mov	r0, DMTIMER3		// r0 -> DMTIMER3 block
	add	r1, r1, 12		// ...and so on
	lbbo	r2, r0, TCLR, 12
	sbco	r2, c24, r1, 12	

	mov	r0, DMTIMER4	
	add	r1, r1, 12	
	lbbo	r2, r0, TCLR, 12
	sbco	r2, c24, r1, 12	

	mov	r0, DMTIMER5	
	add	r1, r1, 12	
	lbbo	r2, r0, TCLR, 12
	sbco	r2, c24, r1, 12	

	mov	r0, DMTIMER6	
	add	r1, r1, 12	
	lbbo	r2, r0, TCLR, 12
	sbco	r2, c24, r1, 12	

	mov	r0, DMTIMER7	
	add	r1, r1, 12	
	lbbo	r2, r0, TCLR, 12
	sbco	r2, c24, r1, 12	

        // Tell host we're done, then halt.
	mov	r31.b0, PRU_R31_VEC_VALID | PRU_EVTOUT_1
	halt


