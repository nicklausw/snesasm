; this is the new test file
; that actually attempts a snes
; skeleton.

; currently won't build.

.lorom
.rombanks 4
.banksize $8000
.bank 0


.org $8000

  clc ; clear carry
  xce ; clear emulation flag

; Clear PPU registers
  ldx #$33
loop: stz $2100,x
  stz $4200,x
  dex
  bpl @loop

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