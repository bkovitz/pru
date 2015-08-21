// Copyright (c) 2014 dhenke@mythopoeic.org
// This is free software -- see COPYING for details.
//
// This is a nearly-minimal PRU program. It delays for five seconds, then
// notifies the host that it has completed, then halts the PRU.
//
// The idea is to have a program that does something you can see from user
// space, without doing anything complicated like playing with IO pins,
// DDR or shared memory.
//
// Try adjusting the DELAYCOUNT value and re-running the test; you should
// be able to convince yourself that the program is actually doing something.

.origin 0 // offset of the start of the code in PRU memory
.entrypoint START // program entry point, used by debugger only

// To signal the host that we're done, we set bit 5 in our R31
// simultaneously with putting the number of the signal we want
// into R31 bits 0-3. See 5.2.2.2 in AM335x PRU-ICSS Reference Guide.
#define PRU0_R31_VEC_VALID (1<<5)
#define SIGNUM 3 // corresponds to PRU_EVTOUT_0

#define DELAY_SECONDS 1 // adjust this to experiment
#define CLOCK 200000000 // PRU is always clocked at 200MHz
#define CLOCKS_PER_LOOP 2 // loop contains two instructions, one clock each
#define DELAYCOUNT DELAY_SECONDS * CLOCK / CLOCKS_PER_LOOP

START:
        // initialize loop counter
	MOV	r1, DELAYCOUNT

        // wait for specified period of time
DELAY:
	SUB	r1, r1, 1     // decrement loop counter
	QBNE	DELAY, r1, 0  // repeat loop unless zero

        // tell host we're done, then halt
	MOV	R31.b0, PRU0_R31_VEC_VALID | SIGNUM
	HALT
