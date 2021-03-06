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
;.org $8000


; label to opcode works simply due to
; how the lexer works. sweet
reset: clc
xce

; opcode args
adc #$12
adc $12
adc $1234
adc $123456

; forward calling labels is kinda
; done? just for db and dw
.dw bla
bla:

; stylistic commas
.dw $1234,$5678,$9ABC
.db $54, $45, $55
.db $01 $02 $03

.org $fffd ; emu reset vector
.dw reset ; labels!

; these give an overflow warning.
; this is the correct functionality.
;.dw $0000
;.dw $0000

; when properly banked, no errors.
; org is reset properly here.
;.bank 1
;.dw $0000


; the end?