#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define	opt_type uint64_t	    //optimal type for 64 bit sys
#define current 0		    	//H (hash values); current is index 0
#define new	1		    		//H (hash values); new is index 1
#define HASH_SIZE 8   		    //H (hash values) array size
#define INIT_BLOCK_SIZE	16		//At first, filled with sixteen 32 bit words of data max.
#define BLOCK_SIZE 64 		    //64 blocks if 32 bit, but since 64 bit only need half space
#define READ_BYTES 4			//num bytes read every fread(), look "readfile" func
#define ML3 0x00000000ff000000  //MoveLeft3, used for "endianswap" func: (num << 24)&ML3
#define ML1 0x0000000000ff0000  //MoveLeft1  (num << 8)&ML1
#define MR1 0x000000000000ff00  //MoveRigth1 (num >> 8)&MR1
#define MR3 0x00000000000000ff  //MoveRight3 (num >> 24)&MR3
#define CLR 0x0					//If extra space, fill up with this.
#define EDGEBLK 14				//Edge case, if more than 14 need one more block, else done
#define PADSEP  0x80			//Extra 1 to seperate between message and bit_size
#define ENSURE32 0x00000000ffffffff //Ensures a 32bit word data with 64 bits

//GLOBALs
//constants H0, ..., H7 aka inital hash values
opt_type H[2][HASH_SIZE] = {
{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19}, //current
{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 
0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19}};//new
//EOGlobals

//Function Declerations
int main(int agrc, char *argv[]);
opt_type ROTR(opt_type x, opt_type rot);
opt_type CH(opt_type x, opt_type y, opt_type z);
opt_type MAJ(opt_type x, opt_type y, opt_type z);
opt_type BSIG0(opt_type x);
opt_type BSIG1(opt_type x);
opt_type SSIG0(opt_type x);
opt_type SSIG1(opt_type x);

char *handlearg(opt_type argc, char *argv[]);
void openfile(FILE **fp, char *fname);
void readfile(opt_type buffer[BLOCK_SIZE], FILE *fp, opt_type output[2]);
opt_type getready_readoutput(opt_type output[2], opt_type buffer[BLOCK_SIZE], opt_type byte_counter);
void padset(opt_type output[2], opt_type buffer[BLOCK_SIZE], opt_type byte_counter);  
void endianswap(opt_type *buffer_index); //custom as should only swap the "last" 32 bits
void create_64_words(opt_type buffer[BLOCK_SIZE]);
void compression(opt_type buffer[BLOCK_SIZE]);
void display_digest(void);

//SOMEWHERE ask if the last "/n" should be included.
int main(int argc, char *argv[])
{
	FILE *fp;
	opt_type buffer[BLOCK_SIZE], output[2], byte_counter = 0, end = 0;
	openfile(&fp, handlearg(argc, argv));

	while(1)
	{	
		readfile(buffer, fp, output);
		//last_bytes_read + READ_BYTES*(full_loops_done - 1)
		byte_counter += output[0] + READ_BYTES*(output[1] - 1); 
		if (output[0] != READ_BYTES) //not always end if read this point, might still need 1 more blk
			{end = getready_readoutput(output, buffer, byte_counter);}
		for(opt_type i = 0; i < output[1]; i++)
			{endianswap(buffer + i);}
		create_64_words(buffer);
		compression(buffer);
		if (end) {break;}
	}
	display_digest();
	return 0;
}
//EOF

char *handlearg(opt_type argc, char *argv[])
{
	if (argc == 2) {return argv[1];}
	else {printf("No file arguments were provided!\a\n"); exit(42);}
}

void openfile(FILE **fp, char *fname)
{
	*fp = fopen(fname, "rb");
	if (*fp == NULL)
        {printf("Could not open file %s\n\a", fname); exit(42);}
}

//output == [last_bytes_read, last_word(i)]
void readfile(opt_type buffer[BLOCK_SIZE], FILE *fp, opt_type output[2])
{	//reads file in binary, stores binary data into buffers last 4 bytes of each 
	//8 byte index; wrapping 32 bit word in a 64 bit word in the hopes of being faster
	//in a 64 bit system.
    opt_type bytes_read = READ_BYTES; //initiate bytes_read, allow loop run
	opt_type i = 0, j = 0; //j = 0 (later incrimented by 2) to ensure last 4bytes in 8 byte wrd
	//this is a symptom of the little endian byte order, least sig bytes have smaller addresses 
	//than more sig bytes. Sidenote each byte you move "forward" towards the most sig byte, is 
	//only +1 the current pointer (each byte of data is mapped to a single number increment in 
	//adress space aka pointers.)
    while( (bytes_read == READ_BYTES) && (i < INIT_BLOCK_SIZE) )
	{
		//the 2nd arg is the number of bytes to read per.
		bytes_read = fread(((uint32_t *)buffer)+j, 1, READ_BYTES, fp);
		j += 2; i++;
	}
	output[0] = bytes_read; output[1] = i;
	//in big endian the least sig numbers have the largest adress, and the most sig
	//have the smallest. Fundementaly it comes down to where you start counting the
	//significance from, in big endian it does right (bigger address) to left(smaller
	//address) and the opposite is true for little endian
}
//makes sense of output[2]; output == [last_bytes_read, last_word(i)]
opt_type getready_readoutput(opt_type output[2], opt_type buffer[BLOCK_SIZE], opt_type byte_counter)
{
	static opt_type padset_count = 0;
	byte_counter *= 8; //convert to bit_counter
	if (padset_count) //must mean we are definetly in the edge case extra last buffer block
		{memset(buffer, CLR, sizeof(opt_type)*INIT_BLOCK_SIZE);} //clear all the memory prepring for the next steps.
	else //ensure that we are not doing the operation more than once.
		{padset(output, buffer, byte_counter); padset_count++;}
	
	if (output[1] > EDGEBLK)
		{return 0;} //not end yet
	else //output[1] <= EDGEBLK ... this is the last block can add bits size.
	{
		//since little endian, have to pass the larger address (+1) of byte_counter to send over 
		//most sig bytes of byte_counter: if big endian then the the opposite would be true!
		buffer[EDGEBLK] = *((uint32_t *)(&byte_counter) + 1); //most sig 4 byte word
		buffer[EDGEBLK + 1] = *((uint32_t *)(&byte_counter)); //least sig 4 byte word
		return 1; //end is here.
	}
}

void padset(opt_type output[2], opt_type buffer[BLOCK_SIZE], opt_type byte_counter)
{
	memset(((uint8_t *)(buffer + output[1] - 1)+output[0]), PADSEP, 1); 
	/*buffer + ouput[1](last_word) - 1 (since last_word == num of read loops not curr index)
	last_word -1 == last buffer index....Then convert to uint8_t pointer, and increment pointer
	by output[0], aka the last num_bytes_read to reach the byte that needs to be writen to.*/
	memset(((uint8_t *)(buffer + output[1] - 1)+output[0]+1), CLR, READ_BYTES - (output[0] + 1));
	//sets last ending bits in last word (the one where PADSEP was inserted) to zero 
	//consider possibly moving to only setting 32 bit space that is being used in the 64 bit
	//wrapper to zero, as later operation will certainly set those bits to zero
	memset((buffer + output[1]), CLR, sizeof(opt_type)*(INIT_BLOCK_SIZE - output[1]));
}

void create_64_words(opt_type buffer[BLOCK_SIZE])
{
	/*by this point, all the index in buffer have only filled there 4
	least sig bytes, and the most sig bytes are filled with zeros, asured.
	Therefore when passing into SHA256 funcs such as SSIG1, don't need to 
	make sure that the 4 most sig bytes are zeros, becuase they are. This
	was all done in prep in the "getready_readoutput() func!!!!!!!!!!!!!!*/
	for(opt_type i = INIT_BLOCK_SIZE; i < BLOCK_SIZE; i++)
		{buffer[i] = ((SSIG1(buffer[i-2]) + buffer[i-7] + 
					 SSIG0(buffer[i-15]) + buffer[i-16])&(ENSURE32));}
}

void endianswap(opt_type *buffer_index) //also sets unused bits to zero!
{
	*buffer_index = ((ML3)&(*buffer_index << 24)) |
					((ML1)&(*buffer_index << 8 )) |
					((MR1)&(*buffer_index >> 8 )) |
					((MR3)&(*buffer_index >> 24));
}

void compression(opt_type buffer[BLOCK_SIZE])
{
	//Fractional part of the cuberoot of the first 64 prime numbers
	const static opt_type K[BLOCK_SIZE] = { 
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 
	0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 
	0xfc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 
	0x6ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b, 
	0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 
	0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};


	//maps H, used to handle last block tossed could reduce to just an array 
	//who is HASH_SIZE by HASH_SIZE, but I would need to use i%HASH_SIZE to 
	//access the correct column. This is the fastest way (for really large sets).
	//You could also have to for loops each that count up to 8 (excluding)
	//but that would also require to counters. Testing might need to be done to reach
	//a definitive answer.
	const static opt_type H_access[BLOCK_SIZE + 1][HASH_SIZE] = {
	//BLOCK_SIZE rows, HASH_SIZE columns
	{0, 1, 2, 3, 4, 5, 6, 7},
	{7, 0, 1, 2, 3, 4, 5, 6},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{5, 6, 7, 0, 1, 2, 3, 4}, //#0
	{4, 5, 6, 7, 0, 1, 2, 3},
	{3, 4, 5, 6, 7, 0, 1, 2}, 
	{2, 3, 4, 5, 6, 7, 0, 1},
	{1, 2, 3, 4, 5, 6, 7, 0},

	{0, 1, 2, 3, 4, 5, 6, 7},
	{7, 0, 1, 2, 3, 4, 5, 6},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{5, 6, 7, 0, 1, 2, 3, 4}, //#1
	{4, 5, 6, 7, 0, 1, 2, 3},
	{3, 4, 5, 6, 7, 0, 1, 2}, 
	{2, 3, 4, 5, 6, 7, 0, 1},
	{1, 2, 3, 4, 5, 6, 7, 0},

	{0, 1, 2, 3, 4, 5, 6, 7},
	{7, 0, 1, 2, 3, 4, 5, 6},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{5, 6, 7, 0, 1, 2, 3, 4}, //#2
	{4, 5, 6, 7, 0, 1, 2, 3},
	{3, 4, 5, 6, 7, 0, 1, 2}, 
	{2, 3, 4, 5, 6, 7, 0, 1},
	{1, 2, 3, 4, 5, 6, 7, 0},

	{0, 1, 2, 3, 4, 5, 6, 7},
	{7, 0, 1, 2, 3, 4, 5, 6},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{5, 6, 7, 0, 1, 2, 3, 4}, //#3
	{4, 5, 6, 7, 0, 1, 2, 3},
	{3, 4, 5, 6, 7, 0, 1, 2}, 
	{2, 3, 4, 5, 6, 7, 0, 1},
	{1, 2, 3, 4, 5, 6, 7, 0},

	{0, 1, 2, 3, 4, 5, 6, 7},
	{7, 0, 1, 2, 3, 4, 5, 6},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{5, 6, 7, 0, 1, 2, 3, 4}, //#4
	{4, 5, 6, 7, 0, 1, 2, 3},
	{3, 4, 5, 6, 7, 0, 1, 2}, 
	{2, 3, 4, 5, 6, 7, 0, 1},
	{1, 2, 3, 4, 5, 6, 7, 0},

	{0, 1, 2, 3, 4, 5, 6, 7},
	{7, 0, 1, 2, 3, 4, 5, 6},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{5, 6, 7, 0, 1, 2, 3, 4}, //#5
	{4, 5, 6, 7, 0, 1, 2, 3},
	{3, 4, 5, 6, 7, 0, 1, 2}, 
	{2, 3, 4, 5, 6, 7, 0, 1},
	{1, 2, 3, 4, 5, 6, 7, 0},

	{0, 1, 2, 3, 4, 5, 6, 7},
	{7, 0, 1, 2, 3, 4, 5, 6},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{5, 6, 7, 0, 1, 2, 3, 4}, //#6
	{4, 5, 6, 7, 0, 1, 2, 3},
	{3, 4, 5, 6, 7, 0, 1, 2}, 
	{2, 3, 4, 5, 6, 7, 0, 1},
	{1, 2, 3, 4, 5, 6, 7, 0},

	{0, 1, 2, 3, 4, 5, 6, 7},
	{7, 0, 1, 2, 3, 4, 5, 6},
	{6, 7, 0, 1, 2, 3, 4, 5},
	{5, 6, 7, 0, 1, 2, 3, 4}, //#7
	{4, 5, 6, 7, 0, 1, 2, 3},
	{3, 4, 5, 6, 7, 0, 1, 2}, 
	{2, 3, 4, 5, 6, 7, 0, 1},
	{1, 2, 3, 4, 5, 6, 7, 0},

	//last shift, still need to access value
	{0, 1, 2, 3, 4, 5, 6, 7}, //#8
	};
	
	opt_type T[2]; //temp words; part of sha256 algo
	opt_type i = 0;

	while( i < BLOCK_SIZE )
	{	
		T[0]  = BSIG1(H[current][H_access[i][4]]) + 
				CH(H[current][H_access[i][4]], H[current][H_access[i][5]], H[current][H_access[i][6]]) 
				+ H[current][H_access[i][7]]
				+ K[i]
				+ buffer[i];
		T[1]  = BSIG0(H[current][H_access[i][0]]) + 
				MAJ(H[current][H_access[i][0]], H[current][H_access[i][1]], H[current][H_access[i][2]]);
		H[current][H_access[i][7]] = ((ENSURE32)&(T[0] + T[1]));
		i++; //shifted down
		H[current][H_access[i][4]] = ((ENSURE32)&(H[current][H_access[i][4]] + T[0]));
	}
	for(i = 0; i < HASH_SIZE; i++)
		{H[current][i] = ((ENSURE32)&(H[current][i] + H[new][i]));
		H[new][i] = H[current][i];}
}

void display_digest(void)
{
	for(opt_type i = 0; i < HASH_SIZE; i++)
		{printf("%08lx", H[new][i]);}
	printf("\n");
}

opt_type ROTR(opt_type x, opt_type rot) //32 since SHA256 works on 32 bit words
	{return ( (x>>rot) | (x<<(32 - rot)) )&ENSURE32;}

opt_type CH(opt_type x, opt_type y, opt_type z)
	{return ( (x&y) | ((~x)&(z)) )&ENSURE32;}

opt_type MAJ(opt_type x, opt_type y, opt_type z)
	{return ( ((x)&(y | z)) | (y&z) )&ENSURE32;}

opt_type BSIG0(opt_type x)
	{return ( ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22) )&ENSURE32;}

opt_type BSIG1(opt_type x)
	{return ( ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25) )&ENSURE32;}

opt_type SSIG0(opt_type x)
	{return ( ROTR(x, 7) ^ ROTR(x, 18) ^ (x>>3) )&ENSURE32;}

opt_type SSIG1(opt_type x)
	{return ( ROTR(x, 17) ^ ROTR(x, 19) ^ (x>>10) )&ENSURE32;}