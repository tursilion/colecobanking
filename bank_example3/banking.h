// bank switching helpers! To switch banks, we just read the magic address
// bank0 (which is the fixed bank) can be selected with 0xffff
// bank1 can be selected with 0xfffe
// bank2 can be selected with 0xfffd
// ... and so on!

// reference to the global bank indicator
extern unsigned int nBank;

// the actual bank switch commands
#define SWITCH_IN_BANK1	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xfffe); nBank=0xfffe;
#define SWITCH_IN_BANK2	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xfffd); nBank=0xfffd;

// switch to an arbitrary bank
#define SWITCH_IN_OLD_BANK(x) (*(volatile unsigned char*)0)=(*(volatile unsigned char*)x); nBank=x;
