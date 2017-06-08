//	registers for the CPU and PPU. Source : http://wiki.nesdev.com/w/index.php/PPU_registers
#define PPU_CTRL_REGISTER		*((unsigned char*)0x2000)
#define PPU_MASK_REGISTER		*((unsigned char*)0x2001)
#define PPU_STATUS_REGISTER		*((unsigned char*)0x2002)
#define OAM_ADDRESS_REGISTER	*((unsigned char*)0x2003)
#define SCROLL_REGISTER			*((unsigned char*)0x2005)
#define PPU_ADDRESS_REGISTER	*((unsigned char*)0x2006)
#define PPU_DATA_REGISTER		*((unsigned char*)0x2007)
#define OAM_DMA_REGISTER		*((unsigned char*)0x4014)
#define JOYPAD1_REGISTER		*((unsigned char*)0x4016)

//	Joypad buttons
#define JP1_A		128
#define JP1_B		64
#define JP1_SELECT	32
#define JP1_START	16
#define JP1_UP		8
#define JP1_DOWN	4
#define JP1_LEFT	2
#define JP1_RIGHT	1

//	sprite attribute values for flipping
#define SPRITE_VERTICAL_FLIP 64
#define SPRITE_HORIZONTAL_FLIP 128

#define TRUE	1
#define FALSE	0

//	cardinal directions. Used by both paddle and ball
#define NESW	0
#define N		1
#define NE		2
#define E		3
#define SE		4
#define S		5
#define SW		6
#define W		7
#define NW		8

//	limit where ball go off-screen
#define BALL_MAX_X	247
#define BALL_MAX_Y	230

//	GLOBAL VARS
#pragma bss-name(push, "ZEROPAGE")	//	reduced access time
unsigned char NMIFlag;
unsigned char FrameCounter;

unsigned char index;
unsigned char index2;

unsigned char X;
unsigned char Y;

unsigned char level;

unsigned char xPaddle;
unsigned char yPaddle;
unsigned char dirPaddle;
unsigned char speedPaddle;

unsigned char xBall;
unsigned char yBall;
unsigned char dirBall;
unsigned char speedBall;

unsigned char inputStatus;


unsigned char gameover = FALSE;
unsigned char playing;
unsigned char launched;
unsigned char paused;
unsigned char pauseTransition;

#include "DATA.c"

#pragma bss-name(push, "OAM")	//sprite-only memory space
unsigned char SPRITE_TABLE[256];


//	FUNCTION PROTOTYPES
void updatePos(void);
void inputListener(void);
void spriteCollision(void);
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

	for (level = 0; level < sizeof(LEVEL_NB); ++level)
	{
		xPaddle = 0x0;
		yPaddle = 0xD0;
		dirPaddle = E;
		speedPaddle = 4;

		xBall = 0x0C;
		yBall = 0xC8;
		dirBall = E;
		speedBall = 2;
		playing = TRUE;
		launched = FALSE;
		paused = FALSE;
		pauseTransition = FALSE;
		writeBackgroundToPPU();

		while (playing)
		{
			inputListener();
			if (pauseTransition == TRUE && FrameCounter >= 30)
				//FrameCounter is auto incremented in nmi interrupt, see reset.s line 134
				pauseTransition = FALSE;

			if (paused == FALSE)
			{
				updatePos();
				spriteCollision();
				writeSpritesToPPU();
			}
			frameRoutine();
		}
		if (gameover) break;
	}
	if (gameover == FALSE) writeWinToPPU();
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
				gameover = TRUE;
				writeGameOverToPPU();
			}
			else if (xBall >= BALL_MAX_X) dirBall = SW;
			break;
		case SW:
			if (yBall >= BALL_MAX_Y)
			{
				playing = FALSE;
				gameover = TRUE;
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
	for (index = 0; index < sizeof(LEVEL[level]); ++index)
	{
		if (LEVEL[level][index] == FALSE)
		{
			if (xBall <= BRICKS_X[index] + 32 && xBall + 8 >= BRICKS_X[index]
				&& yBall < BRICKS_Y[index] + 16 && yBall + 8 >= BRICKS_Y[index])
			{
				LEVEL[level][index] = TRUE;
				switch (dirBall)
				{
					case NE:
						if (xBall < BRICKS_X[index] && xBall + 8 >= BRICKS_X[index]) dirBall = NW;
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
				--LEVEL_NB[level];
				if (LEVEL_NB[level] == 0) {playing = FALSE;}
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
	//	which tile (see .chr)
	SPRITE_TABLE[index2] = 0;
	++index2;
	//	attributes (which palette + flips)
	SPRITE_TABLE[index2] = 3;
	++index2;
	//	x coord
	SPRITE_TABLE[index2] = xBall;
	++index2;
}

void readInput(void)
{
	inputStatus = 0;
	//	enter 1 and 0 to joypad register before accessing the values
	JOYPAD1_REGISTER = 1;
	JOYPAD1_REGISTER = 0;
	for (index = 8; index > 0; --index)
		//	shifting each bit of data (button on/off) to the left side
		inputStatus = inputStatus | ((JOYPAD1_REGISTER & 1) << index-1);
}

void frameRoutine(void)
{
	//	waitVBlank gives a 60 FPS clock
	waitVBlank();
	//render the sprites
	OAM_ADDRESS_REGISTER = 0;
	OAM_DMA_REGISTER = 2;
	//prev instructions need a scroll reset
	resetScrollRegister();
}

void writeBrickToPPU(void)
{
	//ppu_address is in X, Y
	PPU_ADDRESS_REGISTER = X;
	PPU_ADDRESS_REGISTER = Y;

	if (LEVEL[level][index] == FALSE)
	{
		PPU_DATA_REGISTER = 0x80;
		PPU_DATA_REGISTER = 0x81;
		PPU_DATA_REGISTER = 0x81;
		PPU_DATA_REGISTER = 0x82;
	}
	else
	{
		//0x83 was a defined sprite, and might be in the future
		PPU_DATA_REGISTER = 0x83;
		PPU_DATA_REGISTER = 0x83;
		PPU_DATA_REGISTER = 0x83;
		PPU_DATA_REGISTER = 0x83;
	}

	Y += 32;
	if (Y < 32) ++X;

	PPU_ADDRESS_REGISTER = X;
	PPU_ADDRESS_REGISTER = Y;

	if (LEVEL[level][index] == FALSE)
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
	PPU_ADDRESS_REGISTER = 0x8B;
	for (index = 0; index < sizeof(GAME_OVER_STR); ++index)
		PPU_DATA_REGISTER = GAME_OVER_STR[index];
	resetScrollRegister();
}
void writeWinToPPU(void)
{
	waitVBlank();
	PPU_ADDRESS_REGISTER = 0x22;
	PPU_ADDRESS_REGISTER = 0x8C;
	for (index = 0; index < sizeof(WIN_STR); ++index)
		PPU_DATA_REGISTER = WIN_STR[index];
	resetScrollRegister();
}

void writePauseToPPU(void)
{
	waitVBlank();
	PPU_ADDRESS_REGISTER = 0x23;
	PPU_ADDRESS_REGISTER = 0x8D;
	for (index = 0; index < sizeof(PAUSE_STR); ++index)
	{
		if (paused == FALSE) PPU_DATA_REGISTER = 0;
		else PPU_DATA_REGISTER = PAUSE_STR[index];
	}
	resetScrollRegister();
}

void writeBackgroundToPPU(void)
{
	waitVBlank();
	//	writing while screen on is way longer than off
	turnScreenOff();
	resetScrollRegister();

	// BRICKS
	for (index = 0; index < sizeof(LEVEL[level]); ++index)
	{
		X = HBYTE_ADDRESSES[index];
		Y = LBYTE_ADDRESSES[index];
		writeBrickToPPU();
	}

	//Level number
	PPU_ADDRESS_REGISTER = 0x22;
	PPU_ADDRESS_REGISTER = 0x8C;
	for (index = 0; index < sizeof(LEVEL_STR); ++index)
		PPU_DATA_REGISTER = LEVEL_STR[index];
	PPU_DATA_REGISTER = level + '1';

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
	//	we do this twice for the following reason :
	//	PALETTE contains 4 differents colorsets,
	//	but NES has 4 dedicated to background and 4 dedicated to sprites
	//	as we're not using many colors, we can afford using the 4 colorsets twice
	for(index = 0; index < sizeof(PALETTE); ++index)
		PPU_DATA_REGISTER = PALETTE[index];
	for(index = 0; index < sizeof(PALETTE); ++index)
		PPU_DATA_REGISTER = PALETTE[index];

	//	this tells which colorset to use for the background
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
	//	NMI flah will be incremented by NMI interrupt in reset.s
	while (NMIFlag == 0);
	NMIFlag = 0;
}
