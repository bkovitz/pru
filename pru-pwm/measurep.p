// Measure pulse widths at specified GPIO input.
//
// Monitors by recording the value of the DMTIMER2.TCRR counter register.
// Returns measurements to ARM through the Common struct, defined below.

.origin 0 		// offset of the start of the code in PRU memory
.entrypoint start	// program entry point, used by debugger only

#include "constants.h"

.struct Common	// Memory for communication with host, at start of PRU DATA RAM
	.u32	gpio_base	// base address of GPIO register
	.u8	pin_bit		// which pin to read (0..31)
	.u8	pru_evtout	// which PRU_EVTOUT to signal on exit
	.u8	ignored1
	.u8	ignored2
	.u32	start		// timestamp of start of last pulse
	.u32	end		// timestamp of end of last pulse
.ends

start:
	// Clear STANDBY_INIT in SYSCFG so PRU can access main memory.
	lbco	r0, c4, SYSCFG, 4
	clr	r0, r0, 4
	sbco	r0, c4, SYSCFG, 4

	// Make sure that C24 -> start of PRU DATA RAM
	mov	r0, 0x0
	mov	r1, PRU_ICSS_PRU1_CTRL	// hard-coded PRU unit??
	sbbo	r0, r1, CTBIR0, 1

	// Read info from ARM host
	lbco	r0, c24, 0, SIZE(Common)
	.assign	Common, r0, r3, common

	// Put the GPIO pin into input mode
	mov	r20, GPIO_OE		// r20 = offset of output-enable reg
	lbbo	r10, common.gpio_base, r20, 4  // r10 = current I/O settings
	set	r10, common.pin_bit	     // set the bit => input mode
	sbbo	r10, common.gpio_base, r20, 4  // write new I/O settings

	mov	r28, 1
	lsl	r28, r28, common.pin_bit  // r28 = mask for our bit
	mov	r27, GPIO_DATAIN
	or	r27, common.gpio_base, r27  // r27 -> GPIO input reg

input_is_low:
	lbbo	r10, r27, 0, 4	// read input reg into r10
	and	r10, r10, r28	// is our pin high?
	qbeq	input_is_low, r10, 0  // no, keep sampling

	lbco	common.start, c1, TCRR, 4  // timestamp of start of pulse

input_is_high:
	lbbo	r10, r27, 0, 4	// read input reg into r10
	and	r10, r10, r28	// is our pin low?
	qbne	input_is_high, r10, 0  // yes: keep sampling

	lbco	common.end, c1, TCRR, 4	// timestamp at end of pulse

	sbco	common.start, c24, OFFSET(common.start), 8  // write timestamps
	or	r31.b0, common.pru_evtout, PRU_R31_VEC_VALID // notify ARM
	qba	input_is_low

