const unsigned char TEXT[]={"NEPHOID"};

//	black, gray, lt gray, white
const unsigned char PALETTE[]={
	//	4 background palettes
0x30, 0x20, 0x10, 0x00,	//	0 = universal color ; 1-3 = pal 0
0x30, 0x02, 0x03, 0x04,	//	4 = mirror ; 5 - 7 = pal 1
0x30, 0x0A, 0x0B, 0x0C,	//	8 = mirror ; 9 - B = pal 2
0x30, 0x17, 0x27, 0x28	//	C = mirror ; D - F = pal 3
	//	4 sprites palettes
};
// note, 11 is the default background color = blue

const unsigned char Attrib_Table[]={
0x44, // 0100 0100,
0xbb, // 1011 1011,
0x44, // 0100 0100,
0xbb}; // 1011 1011
