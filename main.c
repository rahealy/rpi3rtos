extern int __bss_beg, __bss_end;
extern void _start(void);

void main(void);

//
//_cpuinit()
// Initialize the CPU state and access for EL1, AARCH64.
//
void _cpuinit(void) {
//If at EL2 drop down to EL1
    unsigned long long  reg0 = 0, reg1 = 0;

    asm volatile ("mrs %0, currentel" : "=r"(reg0)::);
    
    if (0x2 == reg0) { //In EL2. Drop down to EL1
//Set up timers.
        asm volatile (
            "mrs    %0, cnthctl_el2\n"   //Read counter control into variable.
            "orr    %0, %0, #3\n"        //Allow EL1 access to the physical timer & counter
            "msr    cnthctl_el2, %0\n"   //Write updated counter control value.
            "msr    cntvoff_el2, xzr\n"  //Virtual timer offset is 0.
            : "=r"(reg0): "r"(reg0):
        );

//Disable traps to hypervisor (won't be using EL2)
        asm volatile (
            "mov    %0, 0x33ff\n"       //Default 0x33ff is fine.
            "msr    cptr_el2, %0\n"     //Default 0x33ff is fine.
            "msr    hstr_el2, xzr\n"    //Don't trap anything in the hypervisor.
            : : "r"(reg0):
        );

//EL1 will be running in AARCH64 mode.
        asm volatile (
            "mrs    %0, hcr_el2\n"         //Read Hypervisor Configuration Register
            "orr    %0, %0, #0x80000000\n" //Set bit 31 for AARCH64 mode in EL1.
            "msr    hcr_el2, %0\n"         //Write Hypervisor Configuration Register
            : "=r"(reg0): "r"(reg0):
        );

//Set SPSR_EL2
        asm volatile (
            "mov    %0, #0x3C5\n"       //Set D,A,I,F flags. Set return to EL1h.
            "mrs    %1, spsr_el2\n"     //Read Hypervisor Configuration Register
            "orr    %1, %1, %0\n"       //Set flags.
            "msr    spsr_el2, %1\n"     //Write Hypervisor Configuration Register
            : "=r"(reg0), "=r"(reg1)
            : "r"(reg0), "r"(reg1):
        );

//After eret, execution will jump to location in elr_el2.
        asm volatile (
            "msr    elr_el2, %0\n" :: "r"(main) :
        ); 

//After eret stack pointer is set to sp_el1
        asm volatile (
            "msr    sp_el1, %0\n" :: "r"(_start) :
        );

//On ERET go to EL1, set stack pointer to sp_el1 and jump to main()
        asm volatile ("eret\n" :::);
    }

    main();
}

void main(void) {
    int i = 5;
    while(1) {
        i = i + 1;
    }
    return;
}
