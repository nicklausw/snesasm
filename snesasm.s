; this is the file that snesasm should
; eventually be able to parse.

; will uncomment as things begin to work.

; let's set up the rom.
.lorom
.slowrom
.banksize $8000
.rombanks 4

; header
.compcheck ; checksum
.autoromsize ; compute rom size in header
.romsize $7 ; ...or on your own.
;.name "THIS IS A TEST"
.carttype 0 ; rom only
.licenseecode 0
.version 1

; now the code
.bank 0

; the assembler default to this
; but might as well give clarity
.org $8000


; label to opcode works simply due to
; how the lexer works. sweet
reset: clc
xce

; opcode args
adc #$34

; forward calling labels is kinda
; done? just for db and dw
.dw bla
bla:

.org $fffd ; emu reset vector
.dw reset ; labels!



; the end?