; this is the new test file
; that actually attempts a snes
; skeleton.

; thanks blargg for code.

; currently won't build all the way.

; things that need to work:
; proper a and xy size control

; header
.compcheck ; checksum
.name "LEGIT  TEST"
.romsize $7 ; rom size
.carttype 0 ; rom only
.licenseecode 0
.version 1

.lorom
.rombanks 4
.banksize $8000
.bank 0


.org $8000
reset:
  clc ; clear carry
  xce ; clear emulation flag
  
  ; a 8-bit, xy 8-bit
  sep #$30

; Clear PPU registers
  ldx #$33
loop:
  stz $2100,x
  stz $4200,x
  dex
  bpl loop

  ; Set background color to $03E0
  lda #$E0
  sta $2122
  lda #$03
  sta $2122

  ; Maximum screen brightness
  lda #$0F
  sta $2100

forever:
  jmp forever 


; this is a thing.
brl $4443

; stylistic commas
.dw $1234,$5678,$9ABC
.db $54, $45, $55
.db $01 $02 $03

.org $fffd
.dw reset
