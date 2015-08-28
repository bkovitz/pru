// Constants for the ARM 335x Sitara processor, especially the PRU


// Memory-mapped GPIO
#define GPIO1 0x4804c000

// offsets
#define GPIO_OE 0x134
#define GPIO_DATAIN 0x138
#define GPIO_SETDATAOUT 0x190
#define GPIO_CLEARDATAOUT 0x194


// Timers
#define DMTIMER0 0x44e05000
#define DMTIMER1_1MS 0x44e31000
#define DMTIMER2 0x48040000
#define DMTIMER3 0x48042000
#define DMTIMER4 0x48044000
#define DMTIMER5 0x48046000
#define DMTIMER6 0x48048000
#define DMTIMER7 0x4804a000

// offsets
#define TCLR 0x38   // timer control register
#define TCRR 0x3c   // timer counter register
#define TLDR 0x40   // timer load register
#define TTGR 0x44   // timer trigger register



// Interrupts

// To signal the host that we're done, we set bit 5 in our R31
// simultaneously with putting the number of the signal we want
// into R31 bits 0-3. See 5.2.2.2 in AM335x PRU-ICSS Reference Guide.
#define PRU_R31_VEC_VALID (1<<5)
#define PRU_EVTOUT_0 3
#define PRU_EVTOUT_1 4

