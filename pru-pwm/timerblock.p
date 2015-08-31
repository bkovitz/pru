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

	mov	r1, 0		// r1 = offset into pru1_data_mem of where
				//   to store data
	mov	r10, CM_PER	// r10 -> CM_PER block

	lbbo	r2, r0, CM_PER_L4LS_CLKSTCTRL, 4  // r2 = clock status reg.
	sbco	r2, c24, r1, 4	// copy to pru1_data_mem
	add	r1, r1, 4	// r1 = offset of where to put next data
				//   in pru1_data_mem

	mov	r0, DMTIMER0	// r0 -> DMTIMER0 block
	lbbo	r2, r0, TCLR, 12	// r2..r4 = TCLR, TCRR, TLDR
	mov	r5, 0xdeadbeef		// DMTIMER0 has no source control
	sbco	r2, c24, r1, 16		// write to pru1_data_mem
	add	r1, r1, 16

	// This is the good way to access DMTIMER2, exploiting the fact that
	// c1 -> DMTIMER2.
	lbco	r2, c1, TCLR, 12	// r2..r4 = TCLR, TCRR, TLDR
	lbbo	r5, r10, CM_PER_TIMER2_CLKCTRL, 4  // r5 = CLKCTRL reg
	sbco	r2, c24, r1, 16		// write to pru1_data_mem
	add	r1, r1, 16

	mov	r0, DMTIMER3		// r0 -> DMTIMER3 block
	lbbo	r2, r0, TCLR, 12	// r2..r4 = TCLR, TCRR, TLDR
	lbbo	r5, r10, CM_PER_TIMER3_CLKCTRL, 4 // r5 = CLKCTRL reg
	sbco	r2, c24, r1, 16		// and so on...
	add	r1, r1, 16

	mov	r0, DMTIMER4
	lbbo	r2, r0, TCLR, 12
	lbbo	r5, r10, CM_PER_TIMER4_CLKCTRL, 4
	sbco	r2, c24, r1, 16
	add	r1, r1, 16

	mov	r0, DMTIMER5
	lbbo	r2, r0, TCLR, 12
	lbbo	r5, r10, CM_PER_TIMER5_CLKCTRL, 4
	sbco	r2, c24, r1, 16
	add	r1, r1, 16

	mov	r0, DMTIMER6
	lbbo	r2, r0, TCLR, 12
	lbbo	r5, r10, CM_PER_TIMER6_CLKCTRL, 4
	sbco	r2, c24, r1, 16
	add	r1, r1, 16

	mov	r0, DMTIMER7
	lbbo	r2, r0, TCLR, 12
	lbbo	r5, r10, CM_PER_TIMER7_CLKCTRL, 4
	sbco	r2, c24, r1, 16

        // Tell host we're done, then halt.
	mov	r31.b0, PRU_R31_VEC_VALID | PRU_EVTOUT_1
	halt


