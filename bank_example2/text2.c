// define a static string in ROM
const char * const strBank2 = "Running from Bank2";

// external prototype
void writeString(unsigned int adr, const char *p);

void showString2(unsigned int adr) {
	writeString(adr, strBank2);
}
