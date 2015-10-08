// Simple test of PRU0
//
// Adds 1 to first int in DATA RAM.

.origin 0		// offset of the start of the code in PRU memory
.entrypoint START	// program entry point, used by debugger only

// To signal the host that we're done, we set bit 5 in our R31
// simultaneously with putting the number of the signal we want
// into R31 bits 0-3. See 5.2.2.2 in AM335x PRU-ICSS Reference Guide.
#define INTC_STROBE (1<<5)
#define EVTOUT_0 3   // Interrupt vector in R31[0:3]

START:
	MOV	R1, 0x01
	LBCO	R2, C24, 0, 4
	ADD	R3, R2, R1
	SBCO	R3, C24, 0, 4

	MOV	R4, 100000000	// delay 0.5 sec
DELAY:
	SUB	R4, R4, 1	// bump counter
	QBNE	DELAY, R4, 0	// continue if counter != 0

        // tell host we're done, then halt
	MOV	R31.b0, INTC_STROBE | EVTOUT_0
	HALT
