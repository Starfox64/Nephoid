/*	$2000	PPU data address 0 : $2020
	$2020	-30 rows of 32 tiles each
	$2040
	$2060
	$2080
	$20A0
	$20C0
	$20E0
	$2100
	... + 7 lines
	$2200
	... + 7 lines
	$2300
	...
	$2380 last line displayed on emulator

	attribute table : $23C0-$23FF
*/
#define PPU_CTRL_REGISTER		*((unsigned char*)0x2000)
#define PPU_MASK_REGISTER		*((unsigned char*)0x2001)
#define PPU_STATUS_REGISTER		*((unsigned char*)0x2002)
#define OAM_ADDRESS_REGISTER	*((unsigned char*)0x2003)
#define OAM_DATA_REGISTER		*((unsigned char*)0x2004)
#define SCROLL_REGISTER			*((unsigned char*)0x2005)
#define PPU_ADDRESS_REGISTER	*((unsigned char*)0x2006)
#define PPU_DATA_REGISTER		*((unsigned char*)0x2007)
#define OAM_DMA_REGISTER		*((unsigned char*)0x4014)
#define JOYPAD1_REGISTER		*((unsigned char*)0x4016)
#define JOYPAD2_REGISTER		*((unsigned char*)0x4017)

#define JP1_A		128
#define JP1_B		64
#define JP1_SELECT	32
#define JP1_START	16
#define JP1_UP		8
#define JP1_DOWN	4
#define JP1_LEFT	2
#define JP1_RIGHT	1

#define SPRITE_VERTICAL_FLIP 64
#define SPRITE_HORIZONTAL_FLIP 128

//	GLOBAL VARS
#pragma bss-name(push, "ZEROPAGE")	//	reduced access time
unsigned char NMIFlag;

unsigned char index;
unsigned char spriteIndex;

unsigned char xBall = 0x25;
unsigned char yBall = 0x25;

unsigned char xPaddle = 0x0;
unsigned char yPaddle = 0xB0;

unsigned char inputStatus;

#include "DATA.c"


#pragma bss-name(push, "OAM")
unsigned char SPRITE_TABLE[256];


//	FUNCTION PROTOTYPES
void writeSpritesToPPU(void);

	//utils functions
void frameRoutine(void);
void readInput(void);
void writeBackgroundToPPU(void);
void turnScreenOff(void);
void turnScreenOn(void);
void loadPalette(void);
void resetScrollRegister(void);
void waitVBlank(void);

void main (void)
{
	turnScreenOff();


	loadPalette();


	resetScrollRegister();
	turnScreenOn();

	writeBackgroundToPPU();
	while (1)
	{
		frameRoutine();
		readInput();
		if ((inputStatus & JP1_LEFT) != 0) --xPaddle;
		if ((inputStatus & JP1_RIGHT) != 0) ++xPaddle;


		writeSpritesToPPU();
	}
}


void writeSpritesToPPU(void)
{
	spriteIndex = 0;
	for (index = 0; index < 4; ++index)
	{
		SPRITE_TABLE[spriteIndex] = yPaddle;
		++spriteIndex;
		SPRITE_TABLE[spriteIndex] = PADDLE[index];
		++spriteIndex;
		SPRITE_TABLE[spriteIndex] = 1 + SPRITE_VERTICAL_FLIP;
		++spriteIndex;
		SPRITE_TABLE[spriteIndex] = xPaddle + index * 8;
		++spriteIndex;
	}
	index = 16;
	//	y coord
	SPRITE_TABLE[index] = yBall;
	++index;
	//	tile
	SPRITE_TABLE[index] = 0;
	++index;
	//	attributes
	SPRITE_TABLE[index] = 3;
	++index;
	//	x coord
	SPRITE_TABLE[3] = xBall;
	++index;
}

void readInput(void)
{
	JOYPAD1_REGISTER = 1;
	JOYPAD1_REGISTER = 0;
	for (index = 8; index > 0; --index)
	{
		inputStatus = inputStatus | (JOYPAD1_REGISTER << index-1);
	}
}

void frameRoutine(void)
{
	waitVBlank();
	OAM_ADDRESS_REGISTER = 0;
	OAM_DMA_REGISTER = 2;
	resetScrollRegister();
	inputStatus = 0;
}

void writeBackgroundToPPU(void)
{
	waitVBlank();

	PPU_ADDRESS_REGISTER= 0x20;
	PPU_ADDRESS_REGISTER= 0x20;
	for(index = 0; index < sizeof(TEXT); ++index)
	{
		PPU_DATA_REGISTER = TEXT[index];
	}

	PPU_ADDRESS_REGISTER = 0x23;
	PPU_ADDRESS_REGISTER = 0x80;
	for (index = 0; index < sizeof(TEXT); ++index)
	{
		PPU_DATA_REGISTER = TEXT[index];
	}

	resetScrollRegister();
}

void turnScreenOff(void)
{
	PPU_CTRL_REGISTER = 0;
	PPU_MASK_REGISTER = 0;
}

void turnScreenOn(void)
{
	PPU_CTRL_REGISTER = 0x90;
	PPU_MASK_REGISTER = 0x1e;
}

void loadPalette(void)
{
	PPU_ADDRESS_REGISTER= 0x3f;
	PPU_ADDRESS_REGISTER= 0x00;
	for(index = 0; index < sizeof(PALETTE); ++index)
	{
		PPU_DATA_REGISTER = PALETTE[index];
	}
	for(index = 0; index < sizeof(PALETTE); ++index)
	{
		PPU_DATA_REGISTER = PALETTE[index];
	}

	PPU_ADDRESS_REGISTER = 0x23;
	PPU_ADDRESS_REGISTER = 0xda;
	for( index = 0; index < sizeof(Attrib_Table); ++index )
	{
		PPU_DATA_REGISTER = Attrib_Table[index];
	}
}

void resetScrollRegister(void)
{
	PPU_ADDRESS_REGISTER= 0;
	PPU_ADDRESS_REGISTER= 0;
	SCROLL_REGISTER = 0;
	SCROLL_REGISTER = 0;
}

void waitVBlank(void)
{
	while (NMIFlag == 0);
	NMIFlag = 0;
}
