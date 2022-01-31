// trampoline functions and banking data
#include "banking.h"

// tracking variable (global)
unsigned int nBank;

// prototypes
void showString2(unsigned int adr);

void tramp_copyFromBank2(char *dest, const char *src) {
	unsigned int old = nBank;

	SWITCH_IN_BANK2;
	while (*src) {
		*(dest++)=*(src++);
	}
	*dest = '\0';
	SWITCH_IN_OLD_BANK(old);
}

void tramp_displayBank2(unsigned int adr) {
	unsigned int old = nBank;

	SWITCH_IN_BANK2;
	showString2(adr);
	SWITCH_IN_OLD_BANK(old);
}

