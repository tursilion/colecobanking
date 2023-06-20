// makemegacart.cpp : Defines the entry point for the console application.
// see readme.txt
// for SDCC 3.5.0, 9/2/2015

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *fp;
unsigned char buf[256][1024*16];	// room for up to 4MB right now
unsigned char tmpbuf[1024*16];
int nHighestUsed[256];
char szName[256][128];				// names for each section, if tagged
int nNumBanks=1, nCurrentBank=1;
char szLine[128];
int nBankSwitchAreaUsed=false;
struct {
	char name[128];					// name of the segment (ie: _boss1)
	int bank;						// bank it's in (from the last _bankXX label)
} segmentMap[4096];					// there can be many more segments than banks, see if this works.
int nLastSegment = 0;				// loaded from a map or crt0, if presented
bool isBios = false;

// -1 means not found, else 0-255
int mapsearch(const char *s) {
	for (int idx=0; idx<nLastSegment; idx++) {
		if (0 == strcmp(segmentMap[idx].name, s)) {
			return segmentMap[idx].bank;
		}
	}

	return -1;	// not found
}

bool readmap(const char *fn) {
	FILE *fp;
	char buf[128];
	int idx;
	int lastBank = 0;				// default to code bank
	int line = 0;

	fp=fopen(fn, "r");
	if (NULL == fp) {
		return false;
	}

	nLastSegment = 0;

	while (!feof(fp)) {
		if (NULL == fgets(buf, 128, fp)) break;
		++line;
		
		// strip whitespace
		while ((buf[0])&&(buf[0]<=' ')) memmove(buf, buf+1, 127);
		idx=0;
		while (buf[idx]) ++idx;
		--idx;
		while ((idx>0) && (buf[idx] <= ' ')) buf[idx--]='\0';

		// check for area tag
		if (0 == memcmp(buf, ".area", 5)) {
			// magic name is "_bankXX", anything else is just mapped
			// but we can remember the magic names too, for simplicity
			int p=6,q;								// start after the tag
			while ((buf[p])&&(buf[p]<=' ')) ++p;	// skip whitespace
			q=p;									// strip any comments/etc
			while ((buf[q])&&(buf[q]>' ')) ++q;
			buf[q]='\0';
			strcpy(segmentMap[nLastSegment].name, &buf[p]);
			if (strlen(segmentMap[nLastSegment].name) == 0) {
				printf("Error parsing map line %d - no name?\n", line);
				fclose(fp);
				return false;
			}
			if (mapsearch(segmentMap[nLastSegment].name) != -1) {
				printf("Duplicate segment '%s' in map\n", segmentMap[nLastSegment].name);
				return false;
			}
			if (0 == memcmp(segmentMap[nLastSegment].name, "_ENDOFMAP", 9)) {
				// we're done
				break;
			}
			if (0 == memcmp(segmentMap[nLastSegment].name, "_bank", 5)) {
				lastBank = atoi(&segmentMap[nLastSegment].name[5]);
				if (0 == lastBank) {
					printf("Error parsing map line %d, bad _bank index.\n", line);
					fclose(fp);
					return false;
				}
			}
			segmentMap[nLastSegment].bank = lastBank;
			++nLastSegment;
		}
	}

	fclose(fp);

	return true;
}

int main(int argc, char* argv[])
{
	bool bOverrideSize = false;
	int arg;

	if (argc<3) {
		printf("Specify input (ihx) and output (rom) files, optionally add ROM size in KB!\n");
        printf("Use -bios for the BIOS target instead of ROM\n");
		printf("prefix with -map <filename> to parse a segment map for layout.\n");
		return 10;
	}

	arg=1;

    if (0 == strcmp(argv[arg], "-bios")) {
        isBios = true;
		printf("Going to target BIOS memory range.\n");

		++arg;
		if (argc-arg < 2) {
			printf("BIOS set but insufficient arguments for input and output.\n");
			return 10;
		}
	}

    if (0 == strcmp(argv[arg], "-map")) {
		if (!readmap(argv[++arg])) {
			printf("Failed reading map.\n");
			return 10;
		}

		printf("Succeeded reading map, found %d segments\n", nLastSegment);

		++arg;
		if (argc-arg < 2) {
			printf("Map read but insufficient arguments for input and output.\n");
			return 10;
		}
	}

	fp=fopen(argv[arg++], "rb");
	if (NULL == fp) {
		printf("Failed to open file!\n");
		return 10;
	}
	memset(buf, 0xff, sizeof(buf));		// fill with 0xFF for the EPROMs
	memset(szName, 0, sizeof(szName));
	nHighestUsed[0]=0x8000;
    if (isBios) nHighestUsed[0]=0x0000;

	for (int i=1; i<256; i++) {
		nHighestUsed[i]=0xc000;
        if (isBios) nHighestUsed[i]=0x0000;
	}

    bool inBad = false;
    int firstbad = 0, lastbad = 0;
    char currentArea[128]="";
    int adr = 0;
	int extendedAdr = 0;	// we don't WANT extended addresses, but it makes the mapping easier to follow
	while (!feof(fp)) {
		char tmp[10];
		int cnt,type,dat;
		int p,q;

		if (NULL == fgets(szLine, 128, fp)) break;
		if (strlen(szLine)<9) continue;

		// strip whitespace
		while ((szLine[0])&&(szLine[0]<=' ')) memmove(szLine, szLine+1, 127);
		p=0;
		while (szLine[p]) ++p;
		--p;
		while ((p>0) && (szLine[p] <= ' ')) szLine[p--]='\0';

		// check for area header
		if (0 == strncmp(szLine, "#AREA:", 6)) {
            strncpy(currentArea, &szLine[6], 127);
            currentArea[127]=0;
			nCurrentBank=1;		// assume by default
			if (nLastSegment > 0) {
				p=6;										// start after the tag
				while ((szLine[p])&&(szLine[p]<=' ')) ++p;	// skip whitespace
				q=p;										// strip any comments/etc
				while ((szLine[q])&&(szLine[q]>' ')) ++q;
				szLine[q]='\0';

				// try a map lookup
				p = mapsearch(szLine+p);
				if (p > 0) {
					// user defined bank again
					printf("Segment %s in bank %d\n", szLine+6, p);
					nCurrentBank = p;				// NO plus 1 with this concept!
				}
			} else if (0 == strncmp(szLine, "#AREA:_bank", 11)) {
				// user defined bank - old mechanism
				nCurrentBank=atoi(&szLine[11])+1;	// (_bank1 is our bank 2)
			}
			if (nCurrentBank > nNumBanks) {
				nNumBanks=nCurrentBank;
				printf("Creating bank %d for area %s\n", nNumBanks, &szLine[6]);
				if (nNumBanks > 255) {
					printf("Too many banks, aborting.\n");
					fclose(fp);
					return -10;
				}
			}
			continue;
		}
		// check tag
		if (szLine[0] != ':') continue;
		// get header
		tmp[0]=szLine[1];
		tmp[1]=szLine[2];
		tmp[2]='\0';
		if (1 != sscanf(tmp, "%x", &cnt)) {
			printf("parse error: %s\n", szLine);
			continue;
		}
		tmp[0]=szLine[3];
		tmp[1]=szLine[4];
		tmp[2]=szLine[5];
		tmp[3]=szLine[6];
		tmp[4]='\0';
		if (1 != sscanf(tmp, "%x", &adr)) {
			printf("parse error: %s\n", szLine);
			continue;
		}
		tmp[0]=szLine[7];
		tmp[1]=szLine[8];
		tmp[2]='\0';
		if (1 != sscanf(tmp, "%x", &type)) {
			printf("parse error: %s\n", szLine);
			continue;
		}

		if (type == 1) {
			// eof
			break;
		}

		if (type == 4) {
			// extended address in the two big endian data bytes
			tmp[0]=szLine[9];
			tmp[1]=szLine[10];
			tmp[2]=szLine[11];
			tmp[3]=szLine[12];
			tmp[4] = '\0';
			if (1 != sscanf(tmp, "%x", &extendedAdr)) {
				printf("parse error: %s\n", szLine);
				continue;
			}
			if (extendedAdr != 0) {
				printf("Warning: non-zero extended address 0x%x\n", extendedAdr);
			}
			continue;
		}

		if (type != 0) {
			printf("Unsupported record: %s (type %d, want 0)\n", szLine, type);
			continue;
		}

		p=9;
		while (cnt--) {
			int xadr = (extendedAdr<<16)|adr;
			tmp[0]=szLine[p++];
			tmp[1]=szLine[p++];
			tmp[2]='\0';
			if (1 != sscanf(tmp, "%x", &dat)) {
				printf("parse error: %s\n", szLine);
				continue;
			}
			// check where it is
            if ((isBios)&&(xadr >= 0x0000) && (xadr < 0x6000)) {
                if (inBad) {
                    inBad = false;
                    printf("Bad data from %04X - %04X for %s (want 0000-5FFF)\n", firstbad, lastbad, currentArea);
                }
				// bios block (always banked in) (Phoenix BIOS has 16k)
				buf[0][xadr] = dat;
				if (xadr > nHighestUsed[0]) nHighestUsed[0]=xadr;
            } else if ((xadr >= 0x8000) && (xadr < 0xC000) && (!isBios)) {
                if (inBad) {
                    inBad = false;
                    printf("Bad data from %04X - %04X for %s (want 8000-BFFF)\n", firstbad, lastbad, currentArea);
                }
				// boot block (always banked in)
				buf[0][xadr-0x8000] = dat;
				if (xadr > nHighestUsed[0]) nHighestUsed[0]=xadr;
			} else if ((xadr >= 0xc000)&&(!isBios)) {
                if (inBad) {
                    inBad = false;
                    printf("Bad data from %04X - %04X for %s (want C000-FFFF)\n", firstbad, lastbad, currentArea);
                }
				// bank switched area
				buf[nCurrentBank][xadr-0xc000] = dat;
				if (xadr >= 0xFFC0) {
					// we just remember for now, it's only an error if we ARE using a megacart
					nBankSwitchAreaUsed=nCurrentBank;
				}
				if (xadr > nHighestUsed[nCurrentBank]) 
				{
					nHighestUsed[nCurrentBank]=xadr;
				}
			} else {
				// addresses below 0x8000 are not used in Coleco ROMs
                if (!inBad) {
                    inBad = true;
                    firstbad = xadr;
                    lastbad = xadr;
                } else {
                    lastbad = xadr;
                }
			}
			adr++;
		}
	}
    if (inBad) {
        inBad = false;
        printf("Bad data from %04X - %04X for %s (address < 8000?)\n", firstbad, lastbad, currentArea);
    }

	fclose(fp);

	printf("%d banks detected\n", nNumBanks+1);

	if (argc > arg+1) {
		int nSize=atoi(argv[arg+1]);
		// valid values:
		// 128
		// 256
		// 512
		// 1024
		switch (nSize) {
			case 64:
				nNumBanks=3;
				printf("** WARNING: 64k ROM is not valid for Megacart. Valid values are 128/256/512/1024.\n");
				bOverrideSize=true;
				break;

			case 128:
				nNumBanks=7;
				bOverrideSize=true;
				break;

			case 256:
				nNumBanks=15;
				bOverrideSize=true;
				break;

			case 512:
				nNumBanks=31;
				bOverrideSize=true;
				break;

			case 1024:
				nNumBanks=63;
				bOverrideSize=true;
				break;

			case 2048:
				nNumBanks=127;
				bOverrideSize=true;
				printf("** WARNING: 2MB ROM is not valid for Megacart. Valid values are 128/256/512/1024.\n");
				break;

			case 4096:
				nNumBanks=255;
				bOverrideSize=true;
				printf("** WARNING: 4MB ROM is not valid for Megacart. Valid values are 128/256/512/1024.\n");
				break;

			default:
				printf("Invalid ROM size specified. Valid values are 128/256/512/1024.\n");
				printf("64, 2048 and 4096 will also be accepted but are non-standard.\n");
				return 10;
		}

		printf("%dkb ROM Requested\n", nSize);
	}

    if (isBios) {
		printf("Not a megacart - writing Phoenix 24k BIOS.\n");

		fp=fopen(argv[arg], "wb");
		if (NULL == fp) {
			printf("Failed to open output file!\n");
			return 10;
		}

		// we'll make a 24k binary
		fwrite(buf[0], 1, 3*8192, fp);	// boot block
		fclose(fp);

		printf("\n#  SWITCH  ROM_AD   COL_AD  FREE   NAME\n");
		printf("=  ======  =======  ======  =====  ===============\n");

		int fre;
		fre=0x5fff-nHighestUsed[0];     // only 24k in the Phoenix BIOS
    	printf("X  n/a     0x00000  0x0000  %5d  BIOS\n", fre);

        return 0;
	}

	if (nNumBanks < 2) {
		printf("This is not a megacart - writing normal Coleco cart ROM\n");

		fp=fopen(argv[arg], "wb");
		if (NULL == fp) {
			printf("Failed to open output file!\n");
			return 10;
		}

		// we'll make a 32k cart, normal order
		fwrite(buf[0], 1, 16384, fp);	// boot block
		fwrite(buf[1], 1, 16384, fp);	// second half

		fclose(fp);

		printf("\n#  SWITCH  ROM_AD   COL_AD  FREE   NAME\n");
		printf("=  ======  =======  ======  =====  ===============\n");

		int fre;
		fre=0xbfff-nHighestUsed[0];
		fre+=0xffff-nHighestUsed[1];	// no bank switch, so we can use all of it
    	printf("X  n/a     0x00000  0x8000  %5d  BOOT\n", fre);

		return 0;
	}

	if (nBankSwitchAreaUsed > 0) {
		printf("Bad MegaCart - bank %d has code/data above $FFC0, which is for bank switching.\n", nBankSwitchAreaUsed);
		int cnt=0;
		printf("\n#  SWITCH  ROM_AD   COL_AD  FREE   NAME\n");
		printf("=  ======  =======  ======  =====  ===============\n");
		for (int i=nNumBanks; i>=0; i--) {
			int fre;
			if (i == 0) {
				fre=0xbfff-nHighestUsed[0];
			} else {
				// ffbf because ffc0 and up are reserved for the bank switch logic
				fre=0xffbf-nHighestUsed[i];
			}

			for (int k=16383-fre; k>=0; k--) {
				if (k>16383-9) continue;
				if (0 == memcmp(&buf[i][k], "LinkTag:",8)) {
					strcpy(szName[i], (const char*)(&buf[i][k])+8);
				}
			}
			printf("%X  0xFFF%X  0x%05X  %s  %5d  %s\n", i, 0xf-i, cnt*16384, i==0?"0x8000":"0xC000", fre, szName[i]);
			cnt++;
		}
		return -1;
	}

	// else, make sure that the number of banks is a power of 2, -1
	// That is, 7, 15, 31 or 63
	if (!bOverrideSize) {
		int k;
		if (nNumBanks > 127) {
			nNumBanks = 255;
			k=4096;
			printf("Warning: 4MB Megacart is non-standard\n");
		} else if (nNumBanks > 63) {
			nNumBanks = 127;
			k=2048;
			printf("Warning: 2MB Megacart is non-standard\n");
		} else if (nNumBanks > 31) {
			nNumBanks=63;
			k=1024;
		} else if (nNumBanks > 15) {
			nNumBanks=31;
			k=512;
		} else if (nNumBanks > 7) {
			nNumBanks = 15;
			k=256;
		} else {
			// this is the minimum size
			nNumBanks = 7;
			k=128;
		}

		printf("Writing %dk megacart\n", k);
	}

	fp=fopen(argv[arg], "wb");
	if (NULL == fp) {
		printf("Failed to open output file!\n");
		return 10;
	}

	// current theory - the GAL comes up randomly so you can't count
	// on which bank is loaded anyway, so there's no point forcing
	// data into a particular bank, the crt0 needs to be modified to
	// load the code bank. If it's right next to the boot bank as it
	// should be, then this should work before gsinit is called:
	//
	// ld bc,#0xFFFE
	// ld a,(bc)
	//
	// Note if you keep your code segment under 16k, then all the runtime
	// will always be available to all banks.

	// print report header
	printf("\n#  SWITCH  ROM_AD   COL_AD  FREE   NAME\n");
	printf("=  ======  =======  ======  =====  ===============\n");

	int cnt=0;
	for (int i=nNumBanks; i>=0; i--) {
		fwrite(buf[i], 1, 16384, fp);

		int fre;
		if (i == 0) {
			fre=0xbfff-nHighestUsed[0];
		} else {
			// ffbf because ffc0 and up are reserved for the bank switch logic
			fre=0xffbf-nHighestUsed[i];
		}

		for (int k=16383-fre; k>=0; k--) {
			if (k>16383-9) continue;
			if (0 == memcmp(&buf[i][k], "LinkTag:",8)) {
				strcpy(szName[i], (const char*)(&buf[i][k])+8);
			}
		}
//		printf("Bank 0x%X (0xFFF%X) (%s) mapped to %04X has %d bytes free\n", i, 0xf-i, szName[i], cnt*16384, fre);
		printf("%X  0xFFF%X  0x%05X  %s  %5d  %s\n", i, 0xf-i, cnt*16384, i==0?"0x8000":"0xC000", fre, szName[i]);
		cnt++;
	}
	fclose(fp);

	return 0;
}

#if 0
Modifications to link-z80.exe (SDCC)

In as\link\z80\lkrloc.c:

At the top of the file add a reference:
	extern VOID ihxAreaTag(const char *psz);

Add a new variable to VOID relr(VOID):
	static int oldaindex=-1;

In the block "get area pointer", after verifying the valid area index,
add this test for change:
	if (oldaindex != aindex) {
		oldaindex=aindex;
		ihxAreaTag(a[aindex]->a_bap->a_id);
	}

In as\link\z80\lnkihx.c:

Add this new function to the bottom of the file:

	// called when a new area is detected to emit the area name
	VOID ihxAreaTag(const char *psz) {
		fprintf(ofp, "#AREA:%s\n", psz);
	}
#endif
