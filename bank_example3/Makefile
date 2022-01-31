# example2 -- this builds a simple project with three banks
# (fixed, and two code banks) and displays a string from each

CC = sdcc
CFLAGS = -mz80 -c --std-sdcc99 --opt-code-speed
AS = sdasz80
AFLAGS = -plosgff

.PHONY: all clean

# crt0 must be first! (note this is also why the output is named crt0.ihx)
# list all your object files in this variable
# note that the order of the banks is not important in this line
objs = crt0.rel main.rel text1.rel text2.rel trampolines.rel

# this rule calls makemegacart to do the final build of the ROM
all: buildcoleco
	makemegacart.exe -map crt0.s crt0.ihx example3.rom

# this rule links the files into the ihx
buildcoleco: $(objs)
	$(CC) -mz80 --no-std-crt0 $(BANKS) --code-loc 0x8100 --data-loc 0x7000 $(objs)

# banks - all defined the same way, we just need to declare them
BANKS = "-Wl -b_bank1=0xc000" "-Wl -b_bank2=0xc000"

# clean up the build
clean:
	-rm *.rel *.map *.lst *.sym *.asm *.ihx *.rom

# build the crt0 startup code
crt0.rel: crt0.s
	$(AS) $(AFLAGS) crt0.rel crt0.s

# build the source files
main.rel: main.c
	$(CC) $(CFLAGS) -o main.rel main.c --codeseg main --constseg main

text1.rel: text1.c
	$(CC) $(CFLAGS) -o text1.rel text1.c --codeseg text1 --constseg text1

text2.rel: text2.c
	$(CC) $(CFLAGS) -o text2.rel text2.c --codeseg text2 --constseg text2

trampolines.rel: trampolines.c
	$(CC) $(CFLAGS) -o trampolines.rel trampolines.c --codeseg trampolines --constseg trampolines


