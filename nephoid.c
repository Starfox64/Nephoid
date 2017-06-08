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

#define TRUE	1
#define FALSE	0

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
unsigned char FrameCounter;

unsigned char index;
unsigned char index2;

//just used vars
unsigned char X;
unsigned char Y;

unsigned char xPaddle = 0x0;
unsigned char yPaddle = 0xD0;
unsigned char dirPaddle = E;
unsigned char speedPaddle = 4;

unsigned char xBall = 0x0C;
unsigned char yBall = 0xC8;
unsigned char dirBall = E;
unsigned char speedBall = 2;

unsigned char inputStatus;

unsigned char playing = TRUE;
unsigned char launched = FALSE;
unsigned char paused = FALSE;
unsigned char pauseTransition = FALSE;

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
void writeBrickToPPU(void);
void writeBackgroundToPPU(void);
void writePauseToPPU(void);
void writeGameOverToPPU(void);
void writeWinToPPU(void);
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

	while (playing)
	{
		inputListener();
		if (pauseTransition == TRUE && FrameCounter >= 30)
			pauseTransition = FALSE;

		if (paused == FALSE)
		{
			updatePos();
			spriteCollision();
			writeSpritesToPPU();
		}
		frameRoutine();
	}
}

void inputListener(void)
{
	readInput();
	if ((inputStatus & JP1_START) != 0
		&& pauseTransition == FALSE)
	{
		paused = (paused == TRUE)?FALSE:TRUE;
		pauseTransition = TRUE;
		FrameCounter = 0;
		writePauseToPPU();
	}

	if (paused == FALSE)
	{
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
		switch (dirBall) {
			case NE:
				xBall += speedBall;
				yBall -= speedBall;
				break;
			case SE:
				xBall += speedBall;
				yBall += speedBall;
				break;
			case SW:
				xBall -= speedBall;
				yBall += speedBall;
				break;
			case NW:
				xBall -= speedBall;
				yBall -= speedBall;
				break;
		}
	}
}

void spriteCollision(void)
{
	//	PADDLE >< BALL
	if (xPaddle + 4 * 8 >= xBall && xPaddle <= xBall + 8
		&& yPaddle <= yBall + 8 &&  yPaddle + 8 >= yBall)
	{
		if (dirBall == SE) dirBall = NE;
		else dirBall = NW;
	}

	//	BALL >< WALLS
	switch (dirBall) {
		case NE:
			if (xBall >= BALL_MAX_X && yBall <= 8) dirBall = SW;
			else if (yBall <= 8) dirBall = SE;
			else if (xBall >= BALL_MAX_X) dirBall = NW;
			break;
		case SE:
			if (yBall >= BALL_MAX_Y)
			{
				playing = FALSE;
				writeGameOverToPPU();
			}
			else if (xBall >= BALL_MAX_X) dirBall = SW;
			break;
		case SW:
			if (yBall >= BALL_MAX_Y)
			{
				playing = FALSE;
				writeGameOverToPPU();
			}
			else if (xBall <= 0) dirBall = SE;
			break;
		case NW:
			if (xBall <= 0 && yBall <= 8) dirBall = SE;
			else if (yBall <= 8) dirBall = SW;
			else if (xBall <= 0) dirBall = NE;
			break;
	}

	//	BALL >< BRICKS
	for (index = 0; index < sizeof(LEVEL[0]); ++index)
	{
		if (LEVEL[0][index] == FALSE)
		{
			if (xBall <= BRICKS_X[index] + 32 && xBall + 8 >= BRICKS_X[index]
				&& yBall < BRICKS_Y[index] + 16 && yBall + 8 >= BRICKS_Y[index])
			{
				LEVEL[0][index] = TRUE;
				switch (dirBall)
				{
					case NE:
						if (xBall < BRICKS_X[index] && xBall + 8 >= BRICKS_X[index] && yBall < BRICKS_Y[index] + 16 && xBall + 8 ) dirBall = NW;
						else if (xBall < BRICKS_X[index] && xBall + 8 >= BRICKS_X[index]) dirBall = NW;
						else dirBall = SE;
						break;
					case SE:
						if (xBall < BRICKS_X[index] && xBall + 8 >= BRICKS_X[index]) dirBall = SW;
						else dirBall = NE;
						break;
					case SW:
						if (xBall < BRICKS_X[index] + 32 && xBall + 8 >= BRICKS_X[index] + 32) dirBall = SE;
						else dirBall = NW;
						break;
					case NW:
						if (xBall < BRICKS_X[index] + 32 && xBall + 8 >= BRICKS_X[index] + 32) dirBall = NE;
						else dirBall = SW;
						break;
				}
				X = HBYTE_ADDRESSES[index];
				Y = LBYTE_ADDRESSES[index];
				waitVBlank();
				writeBrickToPPU();
				resetScrollRegister();
				--LEVEL_NB[0];
				if (LEVEL_NB[0] == 0) {playing = FALSE; writeWinToPPU();}
				break;
			}
		}
	}

}

//	UTILS FUNCTIONS
void writeSpritesToPPU(void)
{
	index2 = 0;
	for (index = 0; index < sizeof(PADDLE); ++index)
	{
		SPRITE_TABLE[index2] = yPaddle;
		++index2;
		SPRITE_TABLE[index2] = PADDLE[index];
		++index2;
		SPRITE_TABLE[index2] = 1 + ((index==3)?SPRITE_VERTICAL_FLIP:0);
		++index2;
		SPRITE_TABLE[index2] = xPaddle + (index << 3);
		++index2;
	}
	//	y coord
	SPRITE_TABLE[index2] = yBall;
	++index2;
	//	tile
	SPRITE_TABLE[index2] = 0;
	++index2;
	//	attributes
	SPRITE_TABLE[index2] = 3;
	++index2;
	//	x coord
	SPRITE_TABLE[index2] = xBall;
	++index2;
}

void readInput(void)
{
	unsigned char test = 0;//TO DELETE
	// unsigned char address = 0;//TO DELETE
	inputStatus = 0;
	JOYPAD1_REGISTER = 1;
	JOYPAD1_REGISTER = 0;
	for (index = 8; index > 0; --index)
	{
		test = (JOYPAD1_REGISTER & 1);
		// PPU_ADDRESS_REGISTER= 0x20;//TO DELETE
		// PPU_ADDRESS_REGISTER= 0x30 + address;//TO DELETE
		// PPU_DATA_REGISTER = test + '0';//TO DELETE
		inputStatus = inputStatus | (test << index-1);
		// ++address;//TO DELETE
	}
	// resetScrollRegister();//TO DELETE
}

void frameRoutine(void)
{
	waitVBlank();
	OAM_ADDRESS_REGISTER = 0;
	OAM_DMA_REGISTER = 2;
	resetScrollRegister();
}

void writeBrickToPPU(void)
{
	//ppu_address is in X, Y
	PPU_ADDRESS_REGISTER = X;
	PPU_ADDRESS_REGISTER = Y;

	if (LEVEL[0][index] == FALSE)
	{
		PPU_DATA_REGISTER = 0x80;
		PPU_DATA_REGISTER = 0x81;
		PPU_DATA_REGISTER = 0x81;
		PPU_DATA_REGISTER = 0x82;
	}
	else
	{
		PPU_DATA_REGISTER = 0x83;
		PPU_DATA_REGISTER = 0x83;
		PPU_DATA_REGISTER = 0x83;
		PPU_DATA_REGISTER = 0x83;
	}

	Y += 32;
	if (Y < 32) ++X;

	PPU_ADDRESS_REGISTER = X;
	PPU_ADDRESS_REGISTER = Y;

	if (LEVEL[0][index] == FALSE)
	{
		PPU_DATA_REGISTER = 0x90;
		PPU_DATA_REGISTER = 0x91;
		PPU_DATA_REGISTER = 0x91;
		PPU_DATA_REGISTER = 0x92;
	}
	else
	{
		PPU_DATA_REGISTER = 0x83;
		PPU_DATA_REGISTER = 0x83;
		PPU_DATA_REGISTER = 0x83;
		PPU_DATA_REGISTER = 0x83;
	}
}

void writeGameOverToPPU(void)
{
	waitVBlank();
	PPU_ADDRESS_REGISTER = 0x22;
	PPU_ADDRESS_REGISTER = 0x8C;
	for (index = 0; index < sizeof(GAME_OVER); ++index)
		PPU_DATA_REGISTER = GAME_OVER[index];
}
void writeWinToPPU(void)
{
	waitVBlank();
	PPU_ADDRESS_REGISTER = 0x22;
	PPU_ADDRESS_REGISTER = 0x8D;
	for (index = 0; index < sizeof(WIN); ++index)
		PPU_DATA_REGISTER = WIN[index];
}

void writePauseToPPU(void)
{
	waitVBlank();
	PPU_ADDRESS_REGISTER = 0x23;
	PPU_ADDRESS_REGISTER = 0x88;
	for (index = 0; index < sizeof(PAUSE_TEXT); ++index)
	{
		if (paused == FALSE) PPU_DATA_REGISTER = 0;
		else PPU_DATA_REGISTER = PAUSE_TEXT[index];
	}
	resetScrollRegister();
}

void writeBackgroundToPPU(void)
{
	waitVBlank();
	turnScreenOff();
	resetScrollRegister();

	// BRICKS
	for (index = 0; index < sizeof(LEVEL[0]); ++index)
	{
		X = HBYTE_ADDRESSES[index];
		Y = LBYTE_ADDRESSES[index];
		writeBrickToPPU();
	}

	resetScrollRegister();
	turnScreenOn();
	waitVBlank();
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
		PPU_DATA_REGISTER = PALETTE[index];
	for(index = 0; index < sizeof(PALETTE); ++index)
		PPU_DATA_REGISTER = PALETTE[index];

	PPU_ADDRESS_REGISTER = 0x23;
	PPU_ADDRESS_REGISTER = 0xC0;
	for( index = 0; index < sizeof(Attrib_Table); ++index )
		PPU_DATA_REGISTER = Attrib_Table[index];
}

void resetScrollRegister(void)
{
	PPU_ADDRESS_REGISTER = 0;
	PPU_ADDRESS_REGISTER = 0;
	SCROLL_REGISTER = 0;
	SCROLL_REGISTER = 0;
}

void waitVBlank(void)
{
	while (NMIFlag == 0);
	NMIFlag = 0;
}
