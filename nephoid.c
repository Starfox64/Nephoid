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
#define OAM_DMA_REGISTER			*((unsigned char*)0x4014)



//	GLOBAL VARS
#pragma bss-name(push, "ZEROPAGE")	//	reduced access time
unsigned char NMIFlag;
unsigned char index;
unsigned char x;
unsigned char y;


const unsigned char TEXT[]={"NEPHOID"};

//	black, gray, lt gray, white
const unsigned char PALETTE[]={
0x11, 0x00, 0x00, 0x31, // blues
0x00, 0x00, 0x00, 0x15, // red
0x00, 0x00, 0x00, 0x27, // yellow
0x00, 0x00, 0x00, 0x1a, // green
};
// note, 11 is the default background color = blue

const unsigned char Attrib_Table[]={
0x44, // 0100 0100,
0xbb, // 1011 1011,
0x44, // 0100 0100,
0xbb}; // 1011 1011


#pragma bss-name(push, "OAM")
unsigned char SPRITE_TABLE[256];


//	FUNCTION PROTOTYPES
void display(void);
void turnScreenOff(void);
void turnScreenOn(void);
void loadPalette(void);
void resetScrollRegister(void);
void waitVBlank(void);

void main (void)
{
	turnScreenOff();

	x = 0x7;
	y = 0x25;

	loadPalette();

	//	y coord
	SPRITE_TABLE[0] = y;
	//	tile
	SPRITE_TABLE[1] = 0;
	//	attributes (flipping etc)
	SPRITE_TABLE[2] = 0;
	//	x coord
	SPRITE_TABLE[3] = x;

	resetScrollRegister();
	turnScreenOn();

	display();
	while (1)
	{
	waitVBlank();
	OAM_ADDRESS_REGISTER = 0;
	OAM_DMA_REGISTER = 2;
	resetScrollRegister();
	}
};


void display(void)
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

	PPU_ADDRESS_REGISTER = 0x23;
	PPU_ADDRESS_REGISTER = 0xda;
	for( index = 0; index < sizeof(Attrib_Table); ++index ){
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
