; crt0.s for Colecovision cart
	;; Ordering of segments for the linker - copied from sdcc crt0.s
	;; The only purpose of this is so that the linker has seen all the
	;; sections in the order that we want them applied. Addresses are
	;; set by the makefile, or continue on from the last one.
	.area _HOME
	.area _CODE
		.ascii "LinkTag:Fixed\0"	; also to ensure there is data BEFORE the banking LinkTags
		.area _main					;   to work around a bug that drops the first AREA: tag in the ihx

	.area _INITIALIZER
	.area _GSINIT
	.area _GSFINAL

	;; banking (must be located before the RAM sections)
	.area _bank1
		.ascii "LinkTag:Bank1\0"
		.area _text1

	.area _bank2
		.ascii "LinkTag:Bank2\0"
		.area _text2

	;; end of list - needed for makemegacart. Must go before RAM areas.
	; This isn't used by anything else and should not contain data
	.area _ENDOFMAP
			
	;; RAM locations        
	.area _DATA
	.area _BSS
	.area _HEAP

	; Anything you don't define will end up here, so gets put into RAM.
	; MakeMegacart should complain loudly if this happens.

	.module crt0

	.area _HEADER(ABS)
	.org 0x8000

	.db 0x55, 0xaa			; Title screen and 12 second delay - swap 0x55 and 0xaa not to skip it.
	.dw 0								; sprite table stuff? - rarely used by the BIOS as a pointer
	.dw 0								; unknown - rarely used as a pointer to a single byte of RAM by the BIOS.
	.dw 0								; unknown - frequently used in BIOS routines as a pointer to a memory location (data is both written to and read from there, at least 124 bytes are used - maybe this is where the bios routines store most of their data, though with the common value of 0 this would be in the BIOS ROM itself - strange).
	.dw 0								; unknown - rarely used as a pointer to a single byte of RAM by the BIOS.
	.dw _startprog						; where to start execution of program.
	ei		; RST 0x08
	reti					
	ei		; RST 0x10
	reti
	ei		; RST 0x18
	reti
	ei		; RST 0x20
	reti
	ei		; RST 0x28
	reti
	ei		; RST 0x30
	reti
	ei		; RST 0x38 - spinner interrupt
	reti
	jp nmi		; NMI
	.ascii " / / NOT"

_startprog:
	; clear RAM before starting
	ld hl,#0x7000			; set copy source
	ld de,#0x7001			; set copy dest
	ld bc,#0x03ff			; set bytes to copy (1 less than size)
	ld (hl),#0				; set initial value (this gets copied through)
	ldir					; do it
	
	ld  sp, #0x7400			; Set stack pointer directly above top of memory.
	ld	bc,#0xFFFE			; switch in code bank
   	ld	a,(bc)				; note that this does NOT set the local pBank variable, user code still must do that!
	call gsinit				; Initialize global variables. (always at end of code bank, so need above bank switch)
	call _vdpinit			; Initialize something or other ;)
	call _main
	rst 0x0					; Restart when main() returns.

    .area _CODE
nmi:
; *** This is a dummy NMI that doesn't do anything, because we  ***
; *** don't have a library loaded. You should use your own crt0 ***
; *** or copy this function from yours so that interrupts work! ***
	retn

	.area _GSINIT
gsinit::
	.area _GSFINAL
	ret
