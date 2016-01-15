; this is the file that snesasm should
; eventually be able to parse.

; will uncomment as things begin to work.

; let's set up the rom.
;.lorom
;.banksize $8000
;.rombanks 4

; header
;.compcheck ; checksum
;.autoromsize ; compute rom size in header
;.romsize 7 ; ...or on your own.
;.name "THIS IS A TEST"
;.carttype 0 ; rom only
;.licenseecode 0
;.version $01

; now the code
;.bank 0

;clc
;xce
