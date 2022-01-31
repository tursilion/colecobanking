// define a static string in ROM
const char * const strBank1 = "Running from Bank1";

// external prototype
void writeString(unsigned int adr, const char *p);

void showString1(unsigned int adr) {
	writeString(adr, strBank1);
}
