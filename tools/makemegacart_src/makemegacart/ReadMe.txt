makemegacart.cpp : Defines the entry point for the console application.

Getting a little lessy hacky now
Our goal is to read in an Intel Hex format (ihx) file
Code at 0x8000 is the boot code, and goes in the last block of the file
Each block restarting at 0xC000 is a new bank, and counts down from there
The ultimate file size must be a power of 2, at least 128k, a maximum of  1MB.
I've extended the scheme to 4MB for my own potential hardware.
If there are no extra banks, we can just dump a normal order 32k Coleco cart.
Note that matters! The order is different whether it's megacart or not.

NOTE: Has two special requirements and one special feature

First: alternate banks must be in a segment named "bankXX", where 'XX'
is a decimal number for the bank, starting with 1 (the boot block
is assumed unnumbered, and the default code bank is assumed bank0).
All other segments are placed in the default space.

Second: requires a custom modified link-z80.exe which outputs area
names into the ihx file. See the bottom of this file for the modifications
needed.

Bonus: you will get more useful output if you tag the code areas
in your code. Just define a static const char* with the prefix "LINKTAG:xxxx",
where 'xxxx' is whatever you want to see.
ie: static const char *szTag="LinkTag:XTRA";

An updated version lets you pass a simple "linker map", which can be placed in
the crt0, for instance, that shows the order in which segments are to be linked.
Simply define psuedo segments named bankXX, and assign them to 0xC000 in the makefile,
then you can simply place the segments underneath. You can put the LinkTags here
too, which simplifies things, and makes moving files a little easier (you still
need to manually handle the bank switching in your code). For example:
	.area _bank1
		.ascii "LinkTag:Bank1\0"	; make SURE it is NULL terminated
		.area _ssd
		.area _music

	.area _bank2
		.ascii "LinkTag:Bank2\0"
		.area _player
		.area _enemy

	.area _ENDOFMAP			; important! must be before the RAM areas.

My current recommendation is to have your main code bank (containing main, etc)
stay under 16k - this ensures that the program and all runtime files stay in the
fixed bank and never page out (an issue I had to be very conscious of in Mario Bros).
Then the only thing you need to worry about in paged segments is your own code.

Pass the file, which can be a crt0.s or a dedicated text file with the above format,
as "-map <filename>". Only the ".area" directives will be parsed, indentation is ignored
(the ORDER of the entries is what matters).

There is a HUGE gotcha with this. If you use the map concept, then it will assume your
code block is 16k or less, and bank 1 is the /second/ bank on the chip (ie: select 0xFFFE).
AREA:_bank tags are /ignored/ in this case, only the map is used!

If you do NOT, then it assumes your code is 32k (taking up the first and second banks), and
bank 1 is the /third/ bank on the chip (ie: select 0xFFFD). AREA:_bank tags are the ONLY
thing that is looked at.

