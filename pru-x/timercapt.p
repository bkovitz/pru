// Set up DMTIMER5 for GPIO1 event capture and log results to PRU DATA RAM
//
// Raises interrupt PRU_EVTOUT_1 on completion.

.origin 0 		// offset of the start of the code in PRU memory
.entrypoint start	// program entry point, used by debugger only

#include "constants.h"

.struct dmtimer_block
	.u32	_irqstatus_raw
	.u32	_irqstatus
	.u32	_irqenable_set
	.u32	_irqenable_clr
	.u32	_tclr
	.u32	_tcrr
	.u32	_tldr
	.u32	_tcar1
	.u32	_tcar2
	.u32	_clkctrl
	.u32	_clksel
.ends

#define INPUT_PIN 16

start:
	// Clear STANDBY_INIT in SYSCFG so PRU can access main memory.
	lbco	r0, c4, 4, 4
	clr	r0, r0, 4
	sbco	r0, c4, 4, 4

	// Set up DMTIMER5 /////////////////////////////////////////////

	mov	r0, CM_DPLL
	mov	r2, 0x00000001
	sbbo	r2, r0, CLKSEL_TIMER5_CLK, 4  // Set source to CLK_M_OSC

	mov	r0, CM_PER
	mov	r2, 0x00000002
	sbbo	r2, r0, CM_PER_TIMER5_CLKCTRL, 4 // Enable DMTIMER5

	mov	r0, DMTIMER5
	mov	r2, 0x00006303		// Timer pin is input, capture both
	sbbo	r2, r0, TCLR, 4		// events, auto-reload timer

	mov	r0, CM_PER | TIMER_EVT_CAPT
	lbbo	r2, r0, 0, 4
	and	r2.b0, r2.b0, 0xf0
	or	r2.b0, r2.b0, EVENT_GPIO1_2
	sbbo	r2, r0, 0, 4	// timer5_evtcapt = GPIO1, 2 events

	// Put the GPIO1[16] pin into input mode.
	mov	r0, GPIO1 | GPIO_OE
	lbbo	r2, r0, 0, 4	// r2 = current GPIO settings
	set	r2, INPUT_PIN	// set the bit => input mode
	sbbo	r2, r0, 0, 4	// write new GPIO settings

	mov	r0, GPIO1 | GPIO_RISINGDETECT
	lbbo	r2, r0, 0, 4
	set	r2, INPUT_PIN	// enable rising-edge detection
	sbbo	r2, r0, 0, 4

	mov	r0, GPIO1 | GPIO_FALLINGDETECT
	lbbo	r2, r0, 0, 4
	set	r2, INPUT_PIN	// enable falling-edge detection
	sbbo	r2, r0, 0, 4

	/////////////////////////

	mov	r1, 0		// r1 = offset into pru1_data_mem of where
				//   to store data
	mov	r20, CM_PER	// r20 -> CM_PER block
	mov	r21, CM_DPLL	// r21 -> CM_DPLL block
	mov	r22, CONTROL_MODULE // r22 -> CONTROL_MODULE bloxk

	lbbo	r2, r22, CONTROL_STATUS, 4  // r2 = CONTROL_STATUS register
	lbbo	r3, r20, CM_PER_L4LS_CLKSTCTRL, 4  // r3 = clock status reg.
	sbco	r2, c24, r1, 8	// copy to pru1_data_mem
	add	r1, r1, 8	// r1 = offset of where to put next data
				//   in pru1_data_mem
	mov	r0, DMTIMER5
	lbbo	r2, r0, IRQSTATUS_RAW, 16 // r2..r5 = IRQSTATUS_RAW, etc.
	lbbo	r6, r0, TCLR, 12	// r6..r8 = TCLR, TCRR, TLDR
	lbbo	r9, r0, TCAR1, 4	// r9 = TCAR1
	lbbo	r10, r0, TCAR2, 4	// r10 = TCAR2
	lbbo	r11, r20, CM_PER_TIMER5_CLKCTRL, 4
	lbbo	r12, r21, CLKSEL_TIMER5_CLK, 4
	sbco	r2, c24, r1, SIZE(dmtimer_block)

        // Tell host we're done, then halt.
	mov	r31.b0, PRU_R31_VEC_VALID | PRU_EVTOUT_1
	halt

