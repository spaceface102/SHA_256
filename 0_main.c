#include <stdio.h>
#include <stdint.h>


#define	opt_type uint64_t	    //optimal type for 64 bit sys
#define ensure32 0x00000000ffffffff //limit to 32 bit value while using 64 bit word
#define HASH_SIZE 8   		    //H (hash values), eight 32 bit values (256 bits)
#define current 0		    //H (hash values); current is index 0
#define new	1		    //H (hash values); new is index 1
#define BLOCK_SIZE 64 		    //64 blocks in hash

//GLOBAL VALUES/Constants
//constants H0, ..., H7 aka inital hash values
opt_type H[2][HASH_SIZE] = {
{1779033703, 3144134277, 1013904242, 2773480762, //current
 1359893119, 2600822924, 528734635, 1541459225},
{1779033703, 3144134277, 1013904242, 2773480762, //new
 1359893119, 2600822924, 528734635, 1541459225} };

//Fractional part of the cuberoot of the first 64 prime numbers
opt_type K[BLOCK_SIZE] = 
{1116352408, 1899447441, 3049323471, 3921009573, 961987163, 1508970993, 
2453635748, 2870763221, 3624381080, 310598401, 607225278, 1426881987, 
1925078388, 2162078206, 2614888103, 3248222580, 3835390401, 4022224774, 
264347078, 604807628, 770255983, 1249150122, 1555081692, 1996064986, 
2554220882, 2821834349, 2952996808, 3210313671, 3336571891, 3584528711, 
113926993, 338241895, 666307205, 773529912, 1294757372, 1396182291, 
1695183700, 1986661051, 2177026350, 2456956037, 2730485921, 2820302411, 
3259730800, 3345764771, 3516065817, 3600352804, 4094571909, 275423344, 
430227734, 506948616, 659060556, 883997877, 958139571, 1322822218, 
1537002063, 1747873779, 1955562222, 2024104815, 2227730452, 2361852424, 
2428436474, 2756734187, 3204031479, 3329325298};
//EOGlobal

//Function Declerations
int main(void);
opt_type ROTR(opt_type x, opt_type rot);
opt_type CH(opt_type x, opt_type y, opt_type z);
opt_type MAJ(opt_type x, opt_type y, opt_type z);
opt_type BSIG0(opt_type x);
opt_type BSIG1(opt_type x);
opt_type SSIG0(opt_type x);
opt_type SSIG1(opt_type x);
opt_type to32bit(opt_type x);
int main(void)
{
	for(opt_type i = 0; i < 0x100ff; i++)
	{
		printf(" 0x%lx ", i);
	} 
	return 0;
}

void compression(void/*struct block *data, opt_type *hash*/)
{
	//maps H, used to handle last block tossed
	//could reduce to just an array who is 
	//HASH_SIZE by HASH_SIZE, but I would need
	//to use i%HASH_SIZE to access the correct column
	//This is the fastest way.
	static opt_type H_access[BLOCK_SIZE][HASH_SIZE] = {
	//BLOCK_SIZE rows, HASH_SIZE columns
	{0, 1, 2, 3, 4, 5, 6, 7},
	{1, 2, 3, 4, 5, 6, 7, 0},
	{2, 3, 4, 5, 6, 7, 0, 1},
	{3, 4, 5, 6, 7, 0, 1, 2}, //#0
	{4, 5, 6, 7, 0, 1, 2, 3},
	{5, 6, 7, 0, 1, 2, 3, 4},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{7, 0, 1, 2, 3, 4, 5, 6},

	{0, 1, 2, 3, 4, 5, 6, 7},
	{1, 2, 3, 4, 5, 6, 7, 0},
	{2, 3, 4, 5, 6, 7, 0, 1},
	{3, 4, 5, 6, 7, 0, 1, 2}, //#1
	{4, 5, 6, 7, 0, 1, 2, 3},
	{5, 6, 7, 0, 1, 2, 3, 4},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{7, 0, 1, 2, 3, 4, 5, 6},
	
	{0, 1, 2, 3, 4, 5, 6, 7},
	{1, 2, 3, 4, 5, 6, 7, 0},
	{2, 3, 4, 5, 6, 7, 0, 1},
	{3, 4, 5, 6, 7, 0, 1, 2}, //#2
	{4, 5, 6, 7, 0, 1, 2, 3},
	{5, 6, 7, 0, 1, 2, 3, 4},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{7, 0, 1, 2, 3, 4, 5, 6},

	{0, 1, 2, 3, 4, 5, 6, 7},
	{1, 2, 3, 4, 5, 6, 7, 0},
	{2, 3, 4, 5, 6, 7, 0, 1},
	{3, 4, 5, 6, 7, 0, 1, 2}, //#3
	{4, 5, 6, 7, 0, 1, 2, 3},
	{5, 6, 7, 0, 1, 2, 3, 4},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{7, 0, 1, 2, 3, 4, 5, 6},

	{0, 1, 2, 3, 4, 5, 6, 7},
	{1, 2, 3, 4, 5, 6, 7, 0},
	{2, 3, 4, 5, 6, 7, 0, 1},
	{3, 4, 5, 6, 7, 0, 1, 2}, //#4
	{4, 5, 6, 7, 0, 1, 2, 3},
	{5, 6, 7, 0, 1, 2, 3, 4},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{7, 0, 1, 2, 3, 4, 5, 6},
	
	{0, 1, 2, 3, 4, 5, 6, 7},
	{1, 2, 3, 4, 5, 6, 7, 0},
	{2, 3, 4, 5, 6, 7, 0, 1},
	{3, 4, 5, 6, 7, 0, 1, 2}, //#5
	{4, 5, 6, 7, 0, 1, 2, 3},
	{5, 6, 7, 0, 1, 2, 3, 4},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{7, 0, 1, 2, 3, 4, 5, 6},
	
	{0, 1, 2, 3, 4, 5, 6, 7},
	{1, 2, 3, 4, 5, 6, 7, 0},
	{2, 3, 4, 5, 6, 7, 0, 1},
	{3, 4, 5, 6, 7, 0, 1, 2}, //#6
	{4, 5, 6, 7, 0, 1, 2, 3},
	{5, 6, 7, 0, 1, 2, 3, 4},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{7, 0, 1, 2, 3, 4, 5, 6},
	
	{0, 1, 2, 3, 4, 5, 6, 7},
	{1, 2, 3, 4, 5, 6, 7, 0},
	{2, 3, 4, 5, 6, 7, 0, 1},
	{3, 4, 5, 6, 7, 0, 1, 2}, //#7
	{4, 5, 6, 7, 0, 1, 2, 3},
	{5, 6, 7, 0, 1, 2, 3, 4},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{7, 0, 1, 2, 3, 4, 5, 6}
	};
	
	
	opt_type T[2]; //temp words; part of sha algo

	
	if (H_access[60][2] == H_access[42][2]) {printf("True");}
	else {printf("False");}

	//for(opt_type i = 0; i < BLOCK_SIZE; i++)
	//{	
	//	T[0] = BSIG1(hash[new][H_access[i][4]]
	//}



opt_type ROTR(opt_type x, opt_type rot)
	{return ( (x>>rot) | (x<<(32 - rot)) );	}

opt_type CH(opt_type x, opt_type y, opt_type z)
	{return ( (x&y) | ((~x)&(z)) );	}

opt_type MAJ(opt_type x, opt_type y, opt_type z)
	{return ( ((x)&(y | z)) | (y&z) );	}

opt_type BSIG0(opt_type x)
	{return ( ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22) );	}

opt_type BSIG1(opt_type x)
	{return ( ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25) );	}

opt_type SSIG0(opt_type x)
	{return ( ROTR(x, 7) ^ ROTR(x, 18) ^ (x>>3) );	}

opt_type SSIG1(opt_type x)
	{return ( ROTR(x, 17) ^ ROTR(x, 19) ^ (x>>10) );	}

opt_type to32bit(opt_type x)
	{return x&ensure32;}
