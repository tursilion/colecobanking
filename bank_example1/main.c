// example 1 - a simple Hello World with banked data

// bank switching helpers! To switch banks, we just read the magic address
// bank0 (which is the fixed bank) can be selected with 0xffff
// bank1 can be selected with 0xfffe
// bank2 can be selected with 0xfffd
// ... and so on!
#define SWITCH_IN_BANK1	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xfffe);
#define SWITCH_IN_BANK2	(*(volatile unsigned char*)0)=(*(volatile unsigned char*)0xfffd);

// sound access (for initialization)
volatile __sfr __at 0xff SOUND;

inline void MUTE_SOUND() { 
	SOUND=0x9f;		// mute chan 1 
	SOUND=0xbf;		// mute chan 2
	SOUND=0xdf;		// mute chan 3
	SOUND=0xff;		// mute noise 
}

// VDP access
// Read Data
volatile __sfr __at 0xbe VDPRD;
// Read Status
volatile __sfr __at 0xbf VDPST;
// Write Address/Register
volatile __sfr __at 0xbf VDPWA;
// Write Data
volatile __sfr __at 0xbe VDPWD;

// helper for VDP delay between accesses
inline void VDP_SAFE_DELAY() {	
__asm
	nop
	nop
	nop
	nop
	nop
__endasm;
}

// helper for register writes
inline void VDP_SET_REGISTER(unsigned char r, unsigned char v) {
	VDPWA=(v); 
	VDP_SAFE_DELAY();
	VDPWA=(0x80|(r)); 
	VDP_SAFE_DELAY();
}
inline void VDP_SET_ADDRESS_WRITE(unsigned int x) {	
	VDPWA=((x)&0xff); 
	VDP_SAFE_DELAY();
	VDPWA=(((x)>>8)|0x40); 
	VDP_SAFE_DELAY();
}

// now that we are printing multiple strings, just a helper function for that
void writeString(unsigned int adr, const char *p) {
	// Note: this example is not meant to teach Coleco programming, but
	// it is critical that an NMI does not access the VDP while a loop like
	// this executes. This sample is safe because the NMI function is a nop
	// which never clears the VDP status byte. As a result the interrupt
	// fires once and then never again, and even if it did, the NMI handler
	// doesn't touch the VDP, so this remains safe.
	VDP_SET_ADDRESS_WRITE(adr);
	while (*p) {
		VDPWD = *(p++);
		VDP_SAFE_DELAY();
	}
}

// references to the data (which is stored in separate banks!)
extern const char * const strBank1;
extern const char * const strBank2;

// ** Main entry point **
void main() {
	// on entry, the VDP is set up, the audio is muted, so all we need to do is
	// write our strings

	// since the data is in another switched bank, but we are in the fixed bank,
	// all we need to do is select the correct bank before we access the data
	SWITCH_IN_BANK1;
	writeString(0x1800+80+4, strBank1);

	// and same for the other string
	SWITCH_IN_BANK2;
	writeString(0x1800+120+4, strBank2);

	// loop forever, or till reset
	for (;;) { }
}

// the crt0 calls this function to initialize the VDP and sound
// we use it to set up a simple 40 column text mode- setting match 
// the BIOS setup so we don't need to load a character set - if 
// you don't have a BIOS this example won't likely work.
void vdpinit() {
	unsigned int idx;
	const char *p = (const char*)0x15A3;			// address of BIOS character set (assumed)

	// mute the sound chip
	MUTE_SOUND();

	// set up the VDP 
	VDP_SET_REGISTER(0x00, 0);
	VDP_SET_REGISTER(0x01, 0xf0);	// VDP_MODE1_16K | VDP_MODE1_UNBLANK | VDP_MODE1_TEXT | VDP_MODE1_INT
	VDP_SET_REGISTER(0x02, 0x06);	// screen image table at 0x1800
	VDP_SET_REGISTER(0x04, 0x00);	// pattern table at 0x0000
	VDP_SET_REGISTER(0x07, 0xf4);	// white text on dark blue

	// copy in the character set
	VDP_SET_ADDRESS_WRITE(0x0000+0x0100);		// the character set starts with space, 32 chars in
	for (idx = 0; idx < 96*8; idx++) {			// 96 characters to copy
		VDPWD = *(p++);
		VDP_SAFE_DELAY();
	}

	// clear the screen
	VDP_SET_ADDRESS_WRITE(0x1800);
	for (idx = 0; idx<960; idx++) {
		VDPWD = ' ';
		VDP_SAFE_DELAY();
	}
}
