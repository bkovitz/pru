// Look at DMTIMER registers and log them to PRU DATA RAM.
//
// Raises interrupt PRU_EVTOUT_1 on completion.

.origin 0 		// offset of the start of the code in PRU memory
.entrypoint start	// program entry point, used by debugger only

#include "constants.h"

#define NUM_SAMPLES 100

start:
	// Clear STANDBY_INIT in SYSCFG so PRU can access main memory.
	lbco	r0, c4, 4, 4
	clr	r0, r0, 4
	sbco	r0, c4, 4, 4

	mov	r1, DMTIMER0	// r1 -> DMTIMER0 block
	lbbo	r0, r1, TCLR, 4	// r0 = TCLR
	sbco	r0, c24, 0, 4	// data_mem[0] = TCLR
	lbbo	r0, r1, TCRR, 4 // r0 = TCRR
	sbco	r0, c24, 4, 4	// data_mem[1] = TCRR
	lbbo	r0, r1, TLDR, 4	// r0 = TLDR
	sbco	r0, c24, 8, 4	// data_mem[2] = TLDR

	mov	r1, DMTIMER2	// r1 -> DMTIMER2 block
	lbbo	r0, r1, TCLR, 4	// r0 = TCLR
	sbco	r0, c24, 12, 4	// data_mem[3] = TCLR
	lbbo	r0, r1, TCRR, 4 // r0 = TCRR
	sbco	r0, c24, 16, 4	// data_mem[4] = TCRR
	lbbo	r0, r1, TLDR, 4	// r0 = TLDR
	sbco	r0, c24, 20, 4	// data_mem[5] = TLDR

	mov	r1, DMTIMER3	// r1 -> DMTIMER3 block
	lbbo	r0, r1, TCLR, 4	// r0 = TCLR
	sbco	r0, c24, 24, 4	// data_mem[6] = TCLR
	lbbo	r0, r1, TCRR, 4 // r0 = TCRR
	sbco	r0, c24, 28, 4	// data_mem[7] = TCRR
	lbbo	r0, r1, TLDR, 4	// r0 = TLDR
	sbco	r0, c24, 32, 4	// data_mem[8] = TLDR

        // Tell host we're done, then halt.
	mov	r31.b0, PRU_R31_VEC_VALID | PRU_EVTOUT_1
	halt


