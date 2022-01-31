// define a static string in ROM
const char * const strBank2 = "Running from Bank2";

// note: this is a data array. This was a bug I hit while writing this,
// learn from my mistake! If this was a char* like the other string,
// then the VARIABLE is a POINTER, meaning that the pointer address,
// as well as the string, is stored in ROM. When Bank1 tries to access
// the pointer, however, it will get the wrong address because the
// const pointer data will come from bank1 but is stored in bank2.
// However, this is just a global variable with the DATA stored in
// bank 2. The address of a global variable is always available, so
// it's safe. If this is confusing don't worry, you will very rarely
// be using const pointers to const data, I was just trying to
// illustrate a point and sidetracked myself. Arrays of data like
// this one are much more likely.
const char strBank2Copy[] = "Copied from Bank2";

// external prototype
void writeString(unsigned int adr, const char *p);

void showString2(unsigned int adr) {
	writeString(adr, strBank2);
}
