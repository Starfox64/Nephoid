const unsigned char TEXT[]={"NEPHOID"};
const unsigned char PAUSE_TEXT[]={"PAUSED"};
const unsigned char PADDLE[]={1, 2, 2, 1};

const unsigned char LEVEL1_X[]={
	0,32,64,96,
	16,48,80,112,144,
	0,32,64,96
};
const unsigned char LEVEL1_Y[]={
	8,8,8,8,
	24,24,24,24,24,
	56,56,56,56
};
unsigned char LEVEL1_DESTROYED[]={
	1,1,1,1,
	1,1,1,1,1,
	1,1,1,1
};


const unsigned char PALETTE[]={
	//	4 background palettes
	//white lgrey grey dgrey
	0x30, 0x20, 0x10, 0x00,	//	0 = universal color ; 1-3 = pal 0
		//dblue blue dpurple
	0x30, 0x02, 0x03, 0x04,	//	4 = mirror ; 5 - 7 = pal 1
		//dblue, lblue, cblue
	0x30, 0x01, 0x21, 0x3C,	//	8 = mirror ; 9 - B = pal 2
		//orange lorange yellow
	0x30, 0x17, 0x27, 0x28	//	C = mirror ; D - F = pal 3
		//	4 sprites palettes
};

const unsigned char Attrib_Table[]={
	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,

	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,

	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,

	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,
	0x55, 0x55, 0x55, 0x55,
	0xFF, 0xFF, 0xFF, 0xFF

};
