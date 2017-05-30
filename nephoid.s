;
; File generated by cc65 v 2.16 - Git 035baa4
;
	.fopt		compiler,"cc65 v 2.16 - Git 035baa4"
	.setcpu		"6502"
	.smart		on
	.autoimport	on
	.case		on
	.debuginfo	off
	.importzp	sp, sreg, regsave, regbank
	.importzp	tmp1, tmp2, tmp3, tmp4, ptr1, ptr2, ptr3, ptr4
	.macpack	longbranch
	.forceimport	__STARTUP__
	.export		_NMIFlag
	.export		_index
	.export		_x
	.export		_y
	.export		_TEXT
	.export		_PALETTE
	.export		_Attrib_Table
	.export		_SPRITE_TABLE
	.export		_display
	.export		_turnScreenOff
	.export		_turnScreenOn
	.export		_loadPalette
	.export		_resetScrollRegister
	.export		_waitVBlank
	.export		_main

.segment	"RODATA"

_TEXT:
	.byte	$4E,$45,$50,$48,$4F,$49,$44,$00
_PALETTE:
	.byte	$11
	.byte	$00
	.byte	$00
	.byte	$31
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$15
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$27
	.byte	$00
	.byte	$00
	.byte	$00
	.byte	$1A
_Attrib_Table:
	.byte	$44
	.byte	$BB
	.byte	$44
	.byte	$BB

.segment	"BSS"

.segment	"ZEROPAGE"
.segment	"OAM"
.segment	"ZEROPAGE"
_NMIFlag:
	.res	1,$00
.segment	"ZEROPAGE"
_index:
	.res	1,$00
.segment	"ZEROPAGE"
_x:
	.res	1,$00
.segment	"ZEROPAGE"
_y:
	.res	1,$00
.segment	"OAM"
_SPRITE_TABLE:
	.res	256,$00

; ---------------------------------------------------------------
; void __near__ display (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_display: near

.segment	"CODE"

;
; waitVBlank();
;
	jsr     _waitVBlank
;
; PPU_ADDRESS_REGISTER= 0x20;
;
	lda     #$20
	sta     $2006
;
; PPU_ADDRESS_REGISTER= 0x20;
;
	sta     $2006
;
; for(index = 0; index < sizeof(TEXT); ++index)
;
	lda     #$00
	sta     _index
L00B1:	lda     _index
	cmp     #$08
	bcs     L00B2
;
; PPU_DATA_REGISTER = TEXT[index];
;
	ldy     _index
	lda     _TEXT,y
	sta     $2007
;
; for(index = 0; index < sizeof(TEXT); ++index)
;
	inc     _index
	jmp     L00B1
;
; PPU_ADDRESS_REGISTER = 0x23;
;
L00B2:	lda     #$23
	sta     $2006
;
; PPU_ADDRESS_REGISTER = 0x80;
;
	lda     #$80
	sta     $2006
;
; for (index = 0; index < sizeof(TEXT); ++index)
;
	lda     #$00
	sta     _index
L00B3:	lda     _index
	cmp     #$08
	bcs     L0058
;
; PPU_DATA_REGISTER = TEXT[index];
;
	ldy     _index
	lda     _TEXT,y
	sta     $2007
;
; for (index = 0; index < sizeof(TEXT); ++index)
;
	inc     _index
	jmp     L00B3
;
; resetScrollRegister();
;
L0058:	jmp     _resetScrollRegister

.endproc

; ---------------------------------------------------------------
; void __near__ turnScreenOff (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_turnScreenOff: near

.segment	"CODE"

;
; PPU_CTRL_REGISTER = 0;
;
	lda     #$00
	sta     $2000
;
; PPU_MASK_REGISTER = 0;
;
	sta     $2001
;
; }
;
	rts

.endproc

; ---------------------------------------------------------------
; void __near__ turnScreenOn (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_turnScreenOn: near

.segment	"CODE"

;
; PPU_CTRL_REGISTER = 0x90;
;
	lda     #$90
	sta     $2000
;
; PPU_MASK_REGISTER = 0x1e;
;
	lda     #$1E
	sta     $2001
;
; }
;
	rts

.endproc

; ---------------------------------------------------------------
; void __near__ loadPalette (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_loadPalette: near

.segment	"CODE"

;
; PPU_ADDRESS_REGISTER= 0x3f;
;
	lda     #$3F
	sta     $2006
;
; PPU_ADDRESS_REGISTER= 0x00;
;
	lda     #$00
	sta     $2006
;
; for(index = 0; index < sizeof(PALETTE); ++index)
;
	sta     _index
L00B4:	lda     _index
	cmp     #$10
	bcs     L00B5
;
; PPU_DATA_REGISTER = PALETTE[index];
;
	ldy     _index
	lda     _PALETTE,y
	sta     $2007
;
; for(index = 0; index < sizeof(PALETTE); ++index)
;
	inc     _index
	jmp     L00B4
;
; PPU_ADDRESS_REGISTER = 0x23;
;
L00B5:	lda     #$23
	sta     $2006
;
; PPU_ADDRESS_REGISTER = 0xda;
;
	lda     #$DA
	sta     $2006
;
; for( index = 0; index < sizeof(Attrib_Table); ++index ){
;
	lda     #$00
	sta     _index
L00B6:	lda     _index
	cmp     #$04
	bcs     L0090
;
; PPU_DATA_REGISTER = Attrib_Table[index];
;
	ldy     _index
	lda     _Attrib_Table,y
	sta     $2007
;
; for( index = 0; index < sizeof(Attrib_Table); ++index ){
;
	inc     _index
	jmp     L00B6
;
; }
;
L0090:	rts

.endproc

; ---------------------------------------------------------------
; void __near__ resetScrollRegister (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_resetScrollRegister: near

.segment	"CODE"

;
; PPU_ADDRESS_REGISTER= 0;
;
	lda     #$00
	sta     $2006
;
; PPU_ADDRESS_REGISTER= 0;
;
	sta     $2006
;
; SCROLL_REGISTER = 0;
;
	sta     $2005
;
; SCROLL_REGISTER = 0;
;
	sta     $2005
;
; }
;
	rts

.endproc

; ---------------------------------------------------------------
; void __near__ waitVBlank (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_waitVBlank: near

.segment	"CODE"

;
; while (NMIFlag == 0);
;
L00B7:	lda     _NMIFlag
	beq     L00B7
;
; NMIFlag = 0;
;
	lda     #$00
	sta     _NMIFlag
;
; }
;
	rts

.endproc

; ---------------------------------------------------------------
; void __near__ main (void)
; ---------------------------------------------------------------

.segment	"CODE"

.proc	_main: near

.segment	"CODE"

;
; turnScreenOff();
;
	jsr     _turnScreenOff
;
; x = 0x7;
;
	lda     #$07
	sta     _x
;
; y = 0x25;
;
	lda     #$25
	sta     _y
;
; loadPalette();
;
	jsr     _loadPalette
;
; SPRITE_TABLE[0] = y;
;
	lda     _y
	sta     _SPRITE_TABLE
;
; SPRITE_TABLE[1] = 0;
;
	lda     #$00
	sta     _SPRITE_TABLE+1
;
; SPRITE_TABLE[2] = 0;
;
	sta     _SPRITE_TABLE+2
;
; SPRITE_TABLE[3] = x;
;
	lda     _x
	sta     _SPRITE_TABLE+3
;
; resetScrollRegister();
;
	jsr     _resetScrollRegister
;
; turnScreenOn();
;
	jsr     _turnScreenOn
;
; display();
;
	jsr     _display
;
; waitVBlank();
;
L002F:	jsr     _waitVBlank
;
; OAM_ADDRESS_REGISTER = 0;
;
	lda     #$00
	sta     $2003
;
; OAM_DMA_REGISTER = 2;
;
	lda     #$02
	sta     $4014
;
; resetScrollRegister();
;
	jsr     _resetScrollRegister
;
; while (1)
;
	jmp     L002F

.endproc

