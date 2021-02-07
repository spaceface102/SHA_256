#include <stdio.h>
#include <stdlib.h>

//GLOBAL VALUES/Constants
//constants H(0)0, ..., H(0)7 aka inital hash values
unsigned int H[8] = 
	{1779033703, 3144134277, 1013904242, 2773480762,
	 1359893119, 2600822924, 528734635, 1541459225};

//Fractional part of the cuberoot of the first 64 prime numbers
unsigned int K[64] = 
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


//Struct Declerations
struct block;
struct hash;
//EOStruct_Declerations

//Struct Definitions
struct block{
	unsigned int *words;
	struct block *next;
};

struct hash{
	unsigned int current; //current hash value
	unsigned int new; //new hash value after compression
	//contains pointer to all the 'next' hashes,
	//in persepective of the current pointer
	struct hash *access[8]; 
};

//EOStruct_Definitions


//Function Declerations
int main(void);
unsigned int ROTR(unsigned long int x, int rot);
unsigned int CH(unsigned int x, unsigned int y, unsigned int z);
unsigned int MAJ(unsigned int x, unsigned int y, unsigned int z);
unsigned int BSIG0(unsigned int x);
unsigned int BSIG1(unsigned int x);
unsigned int SSIG0(unsigned int x);
unsigned int SSIG1(unsigned int x);

void input_mode(void);
void display_hex(unsigned int *sha_value);
unsigned int *output(struct block *data);
void compression(struct block *data, struct hash *x);
unsigned int find(struct hash *head, unsigned int index);
void file_mode(void);
void create_64_words(unsigned int *words);
struct block *data_input_processing(void);
//EOFunction_Declerations

int main(void)
{
	int d = 3;//decide

	while (!(d == 1 || !d))
	{
		printf("Please choose mode; input mode(0), file mode(1): ");
		scanf("%d%*c", &d); //%*c means ignore the last character aka '\n'
		if (!d){
			input_mode();}
		if (d == 1){
			file_mode();}
	}

	return 0;
}

void input_mode(void)
{
	struct block *data = NULL;
	unsigned int *sha_value;
	printf("Please input your text here: ");
	data = data_input_processing();
	sha_value = output(data);
	display_hex(sha_value);
}

void display_hex(unsigned int *sha_value)
{
	for(int i = 0; i < 8; i++){
		printf("%x", sha_value[i]);}
	printf("\n");
	free(sha_value);
}

unsigned int *output(struct block *data)
{
	struct block *temp;
	struct hash *digest = (struct hash *)malloc(8*sizeof(struct hash));
	unsigned int *out = (unsigned int *)malloc(8*sizeof(unsigned int));
	if (digest == NULL || out == NULL)
		{exit(100);}//ran out of memory
	
	//Initilizing the struct hash digest
	for(int i = 0; i < 8; i++){
		digest[i].current = H[i];
		digest[i].new = H[i];}
	//access contains the "next" pointers to find the next hash
	//worth the computation as lookups will be close to instant 
	//later in the compression cycles, and we avoid having to 
	//create new array when shifting the data off in compression
	for(int i = 0; i < 8; i++)
	{	for(int j = 0; j < 8; j++)
			{digest[i].access[j] = digest + ((i+j)%8);}
	}
	
	//compression
	while(data != NULL)
	{
		compression(data, digest);
		temp = data;	
		data = data->next;
		free(temp->words);
		free(temp);
	}	

	for(int i = 0; i < 8; i++)
		{out[i] = digest[i].current;}
	free(digest);
	return out;
}

void compression(struct block *data, struct hash *x) 
{
	unsigned int T[2];//temporary words
	struct hash *head = x; //to track the current head/first element
	for(int i = 0; i < 64; i++)
	{
		T[0] = BSIG1(find(head, 4)) + 
		       CH(find(head, 4), find(head, 5), find(head, 6)) +
		       find(head, 7) + K[i] + data->words[i];
		T[1] = BSIG0(find(head, 0)) +		     
		       MAJ(find(head, 0), find(head, 1), find(head, 2));
		head->access[7]->new = (unsigned int)(T[0] + T[1]);
		head = head->access[7];
		head->access[4]->new += T[0];  
	}
	
	for(int i = 0; i < 8; i++){
		x[i].current += x[i].new;
		x[i].new = x[i].current;}
}

unsigned int find(struct hash *head, unsigned int index)
	{return head->access[index]->new;}

void file_mode(void)
{
	printf("Comming soon <3!\n");
}

void create_64_words(unsigned int *words)
{	
	for(int i = 16; i < 64; i++){
		words[i] = (unsigned int)(SSIG1(words[i-2]) + words[i-7] +
			   SSIG0(words[i-15]) + words[i-16]);}
}

struct block *data_input_processing(void)
{
	//struct block data will be a linked list
	struct block *data = NULL, *out = NULL, *temp = NULL;
	int condition, marker = 0, j = 0, z = 3;//marker is for edge condition
	int condition2 = 1; //to ensure safe freeing at end of loop
       	unsigned long int b_counter = 0;//b_counter = byte_counter
	unsigned char u;

	u = getchar();
	condition = (int)u != 10;
	if(condition || condition2){//will always run
		data = (struct block*)malloc(sizeof(struct block));
		if (data == NULL)
			{exit(100);}//ran out of memory
		out = data;//this will be the pointer that is returned!
		if (condition)
			{b_counter++;}//since u != '\n' so current u gets conunted
		else{//it is empty, no data in u other than '\n'
			data->words = (unsigned int*)calloc(64, sizeof(unsigned int));
			if (data->words == NULL)
				{exit(100);} //ran out of memory
			data->next = NULL;
			data->words[0] = 1<<31;
			create_64_words(data->words);
			condition2 = 0;}//don't do free code after main loop
	}	

	while(condition){
		temp = data; //used to declare the data->next of current data block
		//allocating space for 64 words
		data->words = (unsigned int*)calloc(64, sizeof(unsigned int));
		if (data->words == NULL)
			{exit(100);}//ran out of memory

		for(j = 0; j < 16; j++)//each block og only have 16 words
		{
			z = 3;
			if (marker)
				{u = 10;}
			if((int)u != 10 ){//done to avoid counting extra byte
				while(z >= 0)
				{
					data->words[j] += ((unsigned int)u)<<(z*8);
					u = getchar();
					z--;
					if((int)u == 10)
						{break;}
					b_counter++;
				}
			}
			if(z >= 0){
				//one more block needs to be allocated, since 
				//there is not enough space for the size of
				//the message to be housed. The next block
				//is marked as the final block with marker = 1
				if (j >= 14){
					data->words[j] += (128<<(z*8));
					z--;
					marker = 1;
					j = 16;}
				
				if(j <= 13)//final major manipulation of function
				{
					//doesn't matter if z >= 0, u is no
					//longer providing information and
					//therefore current words should be 
					//left filled with zeros.
					if (marker == 0){
						data->words[j] += (128<<(z*8)); 
						z--;}
					//can skip words since already 
					//initiated to zero with calloc
					b_counter *= 8;//convert to bits
					data->words[15]=4294967295&(b_counter);
					data->words[14]=4294967295&(b_counter>>32);
					condition = 0;//exit condition for main loop
					j = 16;
				}
			}
		}
		create_64_words(data->words);
		data = (struct block*)malloc(sizeof(struct block));
		if (data == NULL)
			{exit(100);}//ran out of memory
		temp->next = data;
	}
	//since decided to not use if statements to check if new data block is
	//made, there WILL be an extra data block at the end, unless nothing is
	//processed since what was inputed is empty aka, u == 10 at the begining
	if(condition2){
		free(data);
		temp->next = NULL;}

	return out;//out is the first data block
}

unsigned int ROTR(unsigned long int x, int rot)
{	return ( (x>>rot) | (x<<(32 - rot)) );	}

unsigned int CH(unsigned int x, unsigned int y, unsigned int z)
{	return ( (x&y) | ((~x)&(z)) );	}

unsigned int MAJ(unsigned int x, unsigned int y, unsigned int z)
{	return ( ((x)&(y | z)) | (y&z) );	}

unsigned int BSIG0(unsigned int x)
{	return ( ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22) );	}

unsigned int BSIG1(unsigned int x)
{	return ( ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25) );	}

unsigned int SSIG0(unsigned int x)
{	return ( ROTR(x, 7) ^ ROTR(x, 18) ^ (x>>3) );	}

unsigned int SSIG1(unsigned int x)
{	return ( ROTR(x, 17) ^ ROTR(x, 19) ^ (x>>10) );	}
