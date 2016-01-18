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

clc
xce

.org $fffd ; emu reset vector
; to do, turn this into a proper
; label for start of code!
.db $80 $00

; the end?