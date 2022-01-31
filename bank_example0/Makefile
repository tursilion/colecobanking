# example0 -- this just builds a simple, non-bank switched "hello world"
# in order to prove the toolchain and demonstrate a basic project

CC = sdcc
CFLAGS = -mz80 -c --std-sdcc99 --opt-code-speed
AS = sdasz80
AFLAGS = -plosgff

.PHONY: all clean

# crt0 must be first! (note this is also why the output is named crt0.ihx)
# list all your object files in this variable
objs = crt0.rel main.rel

# this rule calls makemegacart to do the final build of the ROM
all: buildcoleco
	makemegacart.exe crt0.ihx example0.rom

# this rule links the files into the ihx
buildcoleco: $(objs)
	$(CC) -mz80 --no-std-crt0 --code-loc 0x8100 --data-loc 0x7000 $(objs)

# clean up the build
clean:
	-rm *.rel *.map *.lst *.sym *.asm *.ihx *.rom

# build the crt0 startup code
crt0.rel: crt0.s
	$(AS) $(AFLAGS) crt0.rel crt0.s

# build the source files
main.rel: main.c
	$(CC) $(CFLAGS) -o main.rel main.c



