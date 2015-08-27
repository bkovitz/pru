// Monitor and log GPIO1[16], which is pin P9.15 on the BeagleBone connectors.
//
// Raises interrupt PRU_EVTOUT_1 on completion. Measures pulse widths
// taken during NUM_SAMPLES pulses, and stores the measurements in an array
// in PRU DATA RAM.

.origin 0 		// offset of the start of the code in PRU memory
.entrypoint start	// program entry point, used by debugger only

#include "gpio.hp"

#define PIN_BIT 16

#define NUM_SAMPLES 100

start:
	// Clear STANDBY_INIT in SYSCFG so PRU can access main memory.
	lbco	r0, c4, 4, 4
	clr	r0, r0, 4
	sbco	r0, c4, 4, 4

	// Put the GPIO1[16] pin into input mode.
	mov	r1, GPIO1 | GPIO_OE
	lbbo	r2, r1, 0, 4	// r2 = current GPIO settings
	set	r2, PIN_BIT	// set the bit => input mode
	sbbo	r2, r1, 0, 4	// write new GPIO settings

	mov	r1, GPIO1 | GPIO_DATAIN  // r1 -> GPIO input register
	mov	r3, (1 << PIN_BIT)  // r3 = mask
	mov	r4, 0		// r4 = offset into DATA RAM of where to
				//   store next sample
	mov	r6, NUM_SAMPLES * 4  // we're done when r4 == r6

main_loop:
	mov	r5, 0		// r5 = duration of pulse measured so far

input_is_low:
	lbbo	r2, r1, 0, 4	// read input register into r2
	and	r2, r2, r3	// is our pin high?
	qbeq	input_is_low, r2, 0  // no, keep sampling

input_is_high:
	add	r5, r5, 1	// bump pulse measurement
	lbbo	r2, r1, 0, 4	// read input register into r2
	and	r2, r2, r3	// is our pin high?
	qbne	input_is_high, r2, 0  // yes, keep sampling

	sbco	r5, c24, r4, 4	// save pulse measurement to DATA RAM
	add	r4, r4, 4	// bump index into measurement array
	qblt	main_loop, r6, r4  // all done?

        // Tell host we're done, then halt.
	mov	r31.b0, PRU_R31_VEC_VALID | PRU_EVTOUT_1
	halt


