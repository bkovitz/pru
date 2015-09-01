// Monitor and log timestamps at specified GPIO input.
//
// Monitors by storing the value of the DMTIMER2.TCRR counter register.

.origin 0 		// offset of the start of the code in PRU memory
.entrypoint start	// program entry point, used by debugger only

#include "constants.h"

.struct Input	// Input from host, at start of PRU DATA RAM
	.u32	gpio_base	// base address of GPIO register
	.u8	pin_bit		// which pin to read (0..31)
	.u8	pru_evtout	// which PRU_EVTOUT to signal on exit
	.u8	num_samples	// number of pulses to read before exiting
.ends

.struct Sample
	.u32	pulse_start
	.u32	pulse_end
.ends

#define SAMPLES_OFFSET ((SIZE(Input) + 3) & 0xfffffffc)

start:
	// Clear STANDBY_INIT in SYSCFG so PRU can access main memory.
	lbco	r0, c4, 4, 4
	clr	r0, r0, 4
	sbco	r0, c4, 4, 4

	// Read info from ARM host
	lbco	r0, c24, 0, SIZE(Input)
	.assign	Input, r0, r1.b2, input

	// Put the GPIO pin into input mode
	mov	r20, GPIO_OE		// r20 = offset of output-enable reg
	lbbo	r2, input.gpio_base, r20, 4  // r2 = current I/O settings
	set	r2, input.pin_bit	     // set the bit => input mode
	sbbo	r2, input.gpio_base, r20, 4  // write new I/O settings

	mov	r29, 0		// r29 = # samples done so far
	mov	r28, 1
	lsl	r28, r28, input.pin_bit  // r28 = mask for our bit
	mov	r27, GPIO_DATAIN
	or	r27, input.gpio_base, r27  // r27 -> GPIO input reg


input_is_low:
	lbbo	r2, r27, 0, 4	// read input reg into r2
	and	r2, r2, r28	// is our pin high?
	qbeq	input_is_low, r2, 0  // no, keep sampling

	lbco	r3, c1, TCRR, 4	 // r3 = DMTIMER2 timestamp at start of pulse

input_is_high:
	lbbo	r2, r27, 0, 4	// read input reg into r2
	and	r2, r2, r28	// is our pin low?
	qbne	input_is_high, r2, 0  // yes: keep sampling

	lbco	r4, c1, TCRR, 4  // r4 = timestamp at end of pulse

	lsl	r5, r29, 3	// r5 = sample count * 8
	add	r5, r5, SAMPLES_OFFSET // r5 = offset of where to store sample
	sbco	r3, c24, r5, 8	// save sample

	mov	r10, 1800000	// This loop doesn't affect the accuracy.
gratuitous_loop:
	sub	r10, r10, 1
	qbne	gratuitous_loop, r10, 0

	add	r29, r29, 1	// bump sample counter
	qblt	input_is_low, input.num_samples, r29.b0

        // Tell host we're done, then halt.
	or	r31.b0, input.pru_evtout, PRU_R31_VEC_VALID
	halt


