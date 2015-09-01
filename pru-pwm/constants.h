// Constants for the ARM 335x Sitara processor, especially the PRU


// Memory-mapped GPIO
#define GPIO1 0x4804c000

// offsets
#define GPIO_OE 0x134
#define GPIO_DATAIN 0x138
#define GPIO_RISINGDETECT 0x148
#define GPIO_FALLINGDETECT 0x14c
#define GPIO_SETDATAOUT 0x190
#define GPIO_CLEARDATAOUT 0x194


// Power, reset, and clock management (PRCM)
#define CM_PER 0x44e00000  // Clock module for peripherals
#define CM_DPLL 0x44e00500 // Clock module for phase-locked loops
#define CONTROL_MODULE 0x44e10000 // Control module (AM335x Tech Ref sec. 9)

//offsets
#define CM_PER_L4LS_CLKSTCTRL 0x00  // L4 clock state control register?
#define CM_PER_TIMER2_CLKCTRL 0x80  // timer activity registers...
#define CM_PER_TIMER3_CLKCTRL 0x84
#define CM_PER_TIMER4_CLKCTRL 0x88
#define CM_PER_TIMER5_CLKCTRL 0xec
#define CM_PER_TIMER6_CLKCTRL 0xf0
#define CM_PER_TIMER7_CLKCTRL 0x7c

#define CLKSEL_TIMER2_CLK 0x08  // timer clock source registers...
#define CLKSEL_TIMER3_CLK 0x0c
#define CLKSEL_TIMER4_CLK 0x10
#define CLKSEL_TIMER5_CLK 0x18
#define CLKSEL_TIMER6_CLK 0x1c
#define CLKSEL_TIMER7_CLK 0x04

#define CONTROL_STATUS 0x40  // contains clock freq info
#define TIMER_EVT_CAPT 0xfd0  // events captured by timers

// events  (from sec. 9.2.4.4.5 of ARM 335x Technical Reference)
#define EVENT_GPIO0_1 17
#define EVENT_GPIO0_2 18
#define EVENT_GPIO1_1 19
#define EVENT_GPIO1_2 20
#define EVENT_GPIO2_1 21
#define EVENT_GPIO2_2 22
#define EVENT_GPIO3_1 23
#define EVENT_GPIO3_2 24


// Timers
#define DMTIMER0 0x44e05000
#define DMTIMER1_1MS 0x44e31000
#define DMTIMER2 0x48040000
#define DMTIMER3 0x48042000
#define DMTIMER4 0x48044000
#define DMTIMER5 0x48046000
#define DMTIMER6 0x48048000
#define DMTIMER7 0x4804a000

// offsets  (from sec. 20.1.5 of ARM 335x Technical Reference)
#define TIDR 0x00           // timer identification register
#define TIOCP_CFG 0x10      // timer OCP config register
#define IRQ_EOI 0x20        // timer IRQ end-of-interrupt register
#define IRQSTATUS_RAW 0x24  // timer status raw register
#define IRQSTATUS 0x28      // timer status register
#define IRQENABLE_SET 0x2c  // timer interrupt enable set register
#define IRQENABLE_CLR 0x30  // timer interrupt enable clear register
#define IRQWAKEEN 0x34      // timer IRQ wake-up enable register
#define TCLR 0x38           // timer control register
#define TCRR 0x3c           // timer counter register
#define TLDR 0x40           // timer load register
#define TTGR 0x44           // timer trigger register
#define TWPS 0x48           // timer write posting bits register
#define TMAR 0x4c           // timer match register
#define TCAR1 0x50          // timer capture register, event 1
#define TSICR 0x54          // timer synchronous interface control register
#define TCAR2 0x58          // timer capture register, event 2


// Interrupts

// To signal the host that we're done, we set bit 5 in our R31
// simultaneously with putting the number of the signal we want
// into R31 bits 0-3. See 5.2.2.2 in AM335x PRU-ICSS Reference Guide.
#define PRU_R31_VEC_VALID (1<<5)
#define PRU_EVTOUT_0 3
#define PRU_EVTOUT_1 4

