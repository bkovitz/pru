// Simple pulse-width modulation (PWM) program for PRU
//
// Sends pulses of specified duration to pin P9.12 on the BeagleBone.
//
// By Ben Kovitz, modified from PRU "Hello World" by Douglas G. Henke
// at http://mythopoeic.org/bbb-pru-minimal/.

.origin 0 // offset of the start of the code in PRU memory
.entrypoint START // program entry point, used by debugger only

// To signal the host that we're done, we set bit 5 in our R31
// simultaneously with putting the number of the signal we want
// into R31 bits 0-3. See 5.2.2.2 in AM335x PRU-ICSS Reference Guide.
#define PRU0_R31_VEC_VALID (1<<5)
#define SIGNUM 3 // corresponds to PRU_EVTOUT_0

#define LO_MICROSECONDS 19000 // adjust this to experiment
#define HI_MICROSECONDS 1000 // adjust this to experiment
#define CLOCK 200000000 // PRU is always clocked at 200MHz
#define CLOCKS_PER_LOOP 2 // loop contains two instructions, one clock each
#define CLOCKS_PER_uS 200 // clock cycles per microsecond
#define CLOCKS_PER_mS CLOCKS_PER_uS * 1000 // clock cycles per millisecond
#define DELAYCOUNT1 LO_MICROSECONDS * CLOCKS_PER_uS / CLOCKS_PER_LOOP
#define DELAYCOUNT2 HI_MICROSECONDS * CLOCKS_PER_uS / CLOCKS_PER_LOOP 

// Memory-mapped GPIO
#define GPIO1 0x4804c000
#define GPIO_OE 0x134
#define GPIO_SETDATAOUT 0x190
#define GPIO_CLEARDATAOUT 0x194

// We'll toggle GPIO1[28], which is P9.12 on the BeagleBone connectors.
#define PIN_BIT 28

#define RUN_MILLISECONDS 5000 // Total duration of program
#define NUM_ITERATIONS \
    (RUN_MILLISECONDS * 1000) / (HI_MICROSECONDS + LO_MICROSECONDS)

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

	MOV	r1, NUM_ITERATIONS	// main loop counter
	MOV	r3, (1 << PIN_BIT)

MAIN_LOOP:
	MOV	r4, GPIO1 | GPIO_SETDATAOUT
	SBBO	r3, r4, 0, 4	// send BIT28 to "set bit" address

	MOV	r2, DELAYCOUNT1	// delay counter
DELAY1:
	SUB	r2, r2, 1	// bump delay counter
	QBNE	DELAY1, r2, 0 	// not done with delay yet?

	MOV	r4, GPIO1 | GPIO_CLEARDATAOUT
	SBBO	r3, r4, 0, 4	// send BIT28 to "clear bit" address

	MOV	r2, DELAYCOUNT2	// delay counter
DELAY2:
	SUB	r2, r2, 1	// bump delay counter
	QBNE	DELAY2, r2, 0	// not done with delay yet?

	SUB	r1, r1, 1	// bump main loop counter
	QBNE	MAIN_LOOP, r1, 0  // more iterations?

        // tell host we're done, then halt
	MOV	R31.b0, PRU0_R31_VEC_VALID | SIGNUM
	HALT
