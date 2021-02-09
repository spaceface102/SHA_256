#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define	opt_type uint64_t	    //optimal type for 64 bit sys
#define current 0		    	//H (hash values); current is index 0
#define new	1		    		//H (hash values); new is index 1
#define HASH_SIZE 8   		    //H (hash values) array size
#define WORD_SIZE 8				//word_size in 64bit architecture is 8 bytes
#define BLOCK_SIZE 32 		    //64 blocks if 32 bit, but since 64 bit only need half space
#define BYTES_PER_BLOCK  (BLOCK_SIZE * WORD_SIZE) //bytes per block
#define ENSUREWORD 0x00000000ffffffff //limit to 32 bit value while using 64 bit native word
#define FILL_BLOCK 0			//file handling, filing buffer with zeros
#define READ_ELEMENTS 16		//only 1

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

void compression(void);
opt_type readfile(opt_type *buffer, char *fname);
void print_buffer(opt_type *buffer, opt_type size);

int main(void)
{
	opt_type buffer[BLOCK_SIZE];
	print_buffer(buffer, BLOCK_SIZE);
	return 0;
}
//EOF

void print_buffer(opt_type *buffer, opt_type size)
{
	for(opt_type i = 0; i < size; i++)
	{
		printf("buffer[%lu] == 0x%lx\n", i, buffer[i]);
	}
}

opt_type readfile(opt_type *buffer, char *fname)
{
	// //to avoid setting buffer to all zeros, a good idea might be
	// //to use bytes_read with the BLOCK_SIZE in order to figure
	// //out what section, if any, needs to be set to 0.
	// FILE *fp;
	// opt_type bytes_read, bytes_notread;
	// fp = fopen("test.txt", "rb");
	// bytes_notread = fread(buffer, sizeof(uint8_t), 4, fp);
	// if (bytes_notread)
	// memset( ((uint8_t *)buffer)+bytes_read, 0, 4*sizeof(opt_type)-bytes_read);
	// fclose(fp);
	// printf("bytes_read == %lu\n", bytes_read);
	// return bytes_read;

	FILE *fp;
	opt_type bytes_notread, bytes_read;
	fp = fopen(fname, "rb");
	if (fp = NULL)
		{printf("Could not open file %s\n\a", fname); exit(42);}
	bytes_read = fread(buffer, sizeof(uint8_t), BLOCK_SIZE, fp);
	bytes_notread = BYTES_PER_BLOCK - bytes_read;
	memset( ((uint8_t *)buffer)+bytes_read, FILL_BLOCK, 
	BLOCK_SIZE*sizeof(opt_type) - bytes_read);
}

void compression(void/*struct block *data, opt_type *hash*/)
{

	//constants H0, ..., H7 aka inital hash values
	//to access intermediate values, use pointer logic ((uint32_t *)H
	static opt_type H64bit[2][HASH_SIZE] = {//x86 version
	{0xbb67ae856a09e667, 0xa54ff53a3c6ef372, 0x9b05688c510e527f, 0x5be0cd191f83d9ab}, //current
	0xbb67ae856a09e667, 0xa54ff53a3c6ef372, 0x9b05688c510e527f, 0x5be0cd191f83d9ab} }; //new
	
	// ORIGINAL, USE IF using 32bits array due to 32 bit architecture
	// static opt_type H[2][HASH_SIZE] = {
	// 	{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 
	// 	0x1f83d9ab, 0x5be0cd19}, {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 
	// 	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19} };
	//HASH_SIZE would be 8 since opt_type woulde be 32 bits

	//Fractional part of the cuberoot of the first 64 prime numbers
	static opt_type K[BLOCK_SIZE] = {//x86 version
	0x71374491428a2f98, 0xe9b5dba5b5c0fbcf, 0x59f111f13956c25b, 
	0xab1c5ed5923f82a4, 0x12835b01d807aa98, 0x550c7dc3243185be, 
	0x80deb1fe72be5d74, 0xc19bf1749bdc06a7, 0xefbe4786e49b69c1, 
	0x240ca1cc0fc19dc6, 0x4a7484aa2de92c6f, 0x76f988da5cb0a9dc, 
	0xa831c66d983e5152, 0xbf597fc7b00327c8, 0xd5a79147c6e00bf3, 
	0x1429296706ca6351, 0x2e1b213827b70a85, 0x53380d134d2c6dfc, 
	0x766a0abb650a7354, 0x92722c8581c2c92e, 0xa81a664ba2bfe8a1, 
	0xc76c51a3c24b8b70, 0xd6990624d192e819, 0x106aa070f40e3585, 
	0x1e376c0819a4c116, 0x34b0bcb52748774c, 0x4ed8aa4a391c0cb3, 
	0x682e6ff35b9cca4f, 0x78a5636f748f82ee, 0x8cc7020884c87814, 
	0xa4506ceb90befffa, 0xc67178f2bef9a3f7}
	//You may note that this version of "K" looks like if you took
	//the "ORIGINAL" that is below, and just combined each index's
	//value together and and then switched their places. This is due
	//to the little endian architecture with x86 CPUs. Still if you 
	//try to access these values with pointer logic, it WILL follow
	//the order that is seen with "ORIGINAL"
	
	//ORIGINAL, USE IF using 32bits array due to 32 bit architecture
	// static opt_type K[BLOCK_SIZE] = { 
	// 0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 
	// 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 
	// 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 
	// 0xfc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 
	// 0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 
	// 0x6ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 
	// 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 
	// 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 
	// 0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 
	// 0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 
	// 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2}
	//BLOCK_SIZE would be 64 since opt_type woulde be 32 bits


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
	
	
	opt_type T[2]; //temp words; part of sha256 algo

	
	if (H_access[11][2] == H_access[19][2]) {printf("True");}
	else {printf("False");}

	//for(opt_type i = 0; i < BLOCK_SIZE; i++)
	//{	
	//	T[0] = BSIG1(hash[new][H_access[i][4]]
	//}
}


opt_type ROTR(opt_type x, opt_type rot)
	{return ( (x>>rot) | (x<<(32 - rot)) );	}

opt_type CH(opt_type x, opt_type y, opt_type z)
	{return ( (x&y) | ((~x)&(z)) );	}

opt_type MAJ(opt_type x, opt_type y, opt_type z)
	{return ( ((x)&(y | z)) | (y&z) );	}

opt_type BSIG0(opt_type x)
	{return ( ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22) );}

opt_type BSIG1(opt_type x)
	{return ( ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25) );}

opt_type SSIG0(opt_type x)
	{return ( ROTR(x, 7) ^ ROTR(x, 18) ^ (x>>3) );}

opt_type SSIG1(opt_type x)
	{return ( ROTR(x, 17) ^ ROTR(x, 19) ^ (x>>10) );}

//if code needs to be ported to different word length architecture
opt_type tosys_word(opt_type x) 
	{return ((x)&(ENSUREWORD));}
