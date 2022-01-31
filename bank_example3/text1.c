// define a static string in ROM
const char * const strBank1 = "Running from Bank1";

// external prototype
void writeString(unsigned int adr, const char *p);
void tramp_copyFromBank2(char *dest, const char *src);
void tramp_displayBank2(unsigned int adr);

// external data
extern const char strBank2Copy[];

// here's our function
void showStrings1() {
	char buf[32];

	// this function is in the fixed bank, and the data is here in bank 1
	writeString(0x1800+80+4, strBank1);

	// this function is in trampoline.c in the fixed bank, and the data is
	// in bank 2 (which that function knows). We get a copy of that data
	// into RAM in 'buf', and write it from there.
	tramp_copyFromBank2(buf, strBank2Copy);
	writeString(0x1800+120+4, buf);

	// this function is in trampoline.c in the fixed bank, and will
	// then call a display function in bank2, then return back to us
	// note how we can still pass on data
	tramp_displayBank2(0x1800+160+4);

}
