// Send multiple PRU_EVTOUT0 interrupts to the ARM.

.origin 0 		// offset of the start of the code in PRU memory
.entrypoint start	// program entry point, used by debugger only

#include "constants.h"

.struct Result	// Data sent back to ARM
	.u32	count		// count of interrupts so far
.ends

start:
	// Clear STANDBY_INIT in SYSCFG so PRU can access main memory.
	lbco	r0, c4, 4, 4
	clr	r0, r0, 4
	sbco	r0, c4, 4, 4

	mov	r0, 1		// r0 = interrupt counter
main_loop:
	sbco	r0, c24, 0, 4	// write to Result.count
	mov	r31.b0, PRU_R31_VEC_VALID | PRU_EVTOUT_0_CODE  // interrupt

//	mov	r10, 10000000
//gratuitous_loop:
//	sub	r10, r10, 1
//	qbne	gratuitous_loop, r10, 0

	add	r0, r0, 1	// bump interrupt counter
	qba	main_loop


