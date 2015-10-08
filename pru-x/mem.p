// Simple test of accessing PRU memory from C

.origin 0 // offset of the start of the code in PRU memory
.entrypoint START // program entry point, used by debugger only

// To signal the host that we're done, we set bit 5 in our R31
// simultaneously with putting the number of the signal we want
// into R31 bits 0-3. See 5.2.2.2 in AM335x PRU-ICSS Reference Guide.
#define PRU0_R31_VEC_VALID (1<<5)
#define SIGNUM 3 // corresponds to PRU_EVTOUT_0

START:
	MOV	R1, 0x01
	LBCO	R2, C24, 0, 4
	ADD	R3, R2, R1
	SBCO	R3, C24, 0, 4

        // tell host we're done, then halt
	MOV	R31.b0, PRU0_R31_VEC_VALID | SIGNUM
	HALT
