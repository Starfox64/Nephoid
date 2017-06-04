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

#define FALSE	1
#define TRUE	0

#define NESW	0
#define N		1
#define NE		2
#define E		3
#define SE		4
#define S		5
#define SW		6
#define W		7
#define NW		8

#define BALL_MAX_X	247
#define BALL_MAX_Y	230

//	GLOBAL VARS
#pragma bss-name(push, "ZEROPAGE")	//	reduced access time
unsigned char NMIFlag;

unsigned char index;
unsigned char spriteIndex;

unsigned char xPaddle = 0x0;
unsigned char yPaddle = 0xB0;
unsigned char dirPaddle = E;
unsigned char speedPaddle = 2;

unsigned char xBall = 0x0C;
unsigned char yBall = 0xA8;
unsigned char dirBall = E;
unsigned char speedBall = 2;

unsigned char inputStatus;

unsigned char launched = FALSE;

#include "DATA.c"

#pragma bss-name(push, "OAM")
unsigned char SPRITE_TABLE[256];


//	FUNCTION PROTOTYPES
void updatePos(void);
void inputListener(void);
void spriteCollision(void);

	//utils functions
void frameRoutine(void);
void readInput(void);
void writeBackgroundToPPU(void);
void writeSpritesToPPU(void);
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
		inputListener();

		updatePos();
		spriteCollision();
		writeSpritesToPPU();
	}
}

void inputListener(void)
{
	readInput();
	if ((inputStatus & JP1_LEFT) != 0)
	{
		dirPaddle = W;
		if (launched == FALSE) dirBall = NW;
	}
	else if ((inputStatus & JP1_RIGHT) != 0) //255-size of paddle
	{
		dirPaddle = E;
		if (launched == FALSE) dirBall = NE;
	}
	else dirPaddle = NESW;

	if ((inputStatus & JP1_A) != 0 && launched == FALSE)
		launched = TRUE;
}

void updatePos(void)
{
	//	PADDLE
	if (dirPaddle == E && xPaddle < 223) {
		xPaddle += speedPaddle;
		if (launched == FALSE) xBall += speedPaddle;
	}
	else if (dirPaddle == W && xPaddle > 0) {
		xPaddle -= speedPaddle;
		if (launched == FALSE) xBall -= speedPaddle;
	}
	//	BALL
	if (launched == TRUE) {
		moveAgain:
		switch (dirBall) {
			case NE:
				if (xBall < BALL_MAX_X && yBall > 0)
				{xBall += speedBall;	yBall -= speedBall;}
				else {
					if (xBall >= BALL_MAX_X && yBall <= 0) dirBall = SW;
					else if (yBall <= 0) dirBall = SE;
					else dirBall = NW;
					goto moveAgain;
				}
				break;
			case SE:
				if (xBall < BALL_MAX_X && yBall < BALL_MAX_Y)
				{xBall += speedBall;	yBall += speedBall;}
				else {
					if (xBall >= BALL_MAX_X && yBall >= BALL_MAX_Y) dirBall = NW;
					else if (yBall >= BALL_MAX_Y) dirBall = NE;
					else dirBall = SW;
					goto moveAgain;
				}
				break;
			case SW:
				if (xBall > 0 && yBall < BALL_MAX_Y)
				{xBall -= speedBall;	yBall += speedBall;}
				else {
					if (xBall <= 0 && yBall >= BALL_MAX_Y) dirBall = NE;
					else if (yBall >= BALL_MAX_Y) dirBall = NW;
					else dirBall = SE;
					goto moveAgain;
				}
				break;
			case NW:
				if (xBall > 0 && yBall > 0)
				{xBall -= speedBall;	yBall -= speedBall;}
				else {
					if (xBall <= 0 && yBall <= 0) dirBall = SE;
					else if (yBall <= 0) dirBall = SW;
					else dirBall = NE;
					goto moveAgain;
				}
				break;
		}
	}
}

void spriteCollision(void)
{
	if (xPaddle + 4 * 8 >= xBall && xPaddle <= xBall + 8
		&& yPaddle <= yBall + 8 &&  yPaddle + 8 >= yBall)
	{
		if (dirBall == SE) dirBall = NE;
		else dirBall = NW;
	}
}

//	UTILS FUNCTIONS
void writeSpritesToPPU(void)
{
	spriteIndex = 0;
	for (index = 0; index < 4; ++index)
	{
		SPRITE_TABLE[spriteIndex] = yPaddle;
		++spriteIndex;
		SPRITE_TABLE[spriteIndex] = PADDLE[index];
		++spriteIndex;
		SPRITE_TABLE[spriteIndex] = 1 + ((index==3)?SPRITE_VERTICAL_FLIP:0);
		++spriteIndex;
		SPRITE_TABLE[spriteIndex] = xPaddle + (index << 3);
		++spriteIndex;
	}
	//	y coord
	SPRITE_TABLE[spriteIndex] = yBall;
	++spriteIndex;
	//	tile
	SPRITE_TABLE[spriteIndex] = 0;
	++spriteIndex;
	//	attributes
	SPRITE_TABLE[spriteIndex] = 3;
	++spriteIndex;
	//	x coord
	SPRITE_TABLE[spriteIndex] = xBall;
	++spriteIndex;
}

void readInput(void)
{
	unsigned char test = 0;//TO DELETE
	unsigned char address = 0;//TO DELETE
	inputStatus = 0;
	JOYPAD1_REGISTER = 1;
	JOYPAD1_REGISTER = 0;
	for (index = 8; index > 0; --index)
	{
		test = (JOYPAD1_REGISTER & 1);
		PPU_ADDRESS_REGISTER= 0x20;//TO DELETE
		PPU_ADDRESS_REGISTER= 0x30 + address;//TO DELETE
		PPU_DATA_REGISTER = test + '0';//TO DELETE
		inputStatus = inputStatus | (test << index-1);
		++address;//TO DELETE
	}
	resetScrollRegister();//TO DELETE
}

void frameRoutine(void)
{
	waitVBlank();
	OAM_ADDRESS_REGISTER = 0;
	OAM_DMA_REGISTER = 2;
	resetScrollRegister();
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
