#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define filesize 9 //bytes "abcdefgh\n"

int main(void)
{
    // FILE *fp;
    // uint64_t buffer[2];
    // char fname[] = "file_test.txt";
    // fp = fopen(fname, "rb");
    // if (fp == NULL)
    //     {printf("Could not open file %s\n\a", fname); exit(42);}
    // fread(buffer, sizeof(uint8_t), filesize, fp);
    // fclose(fp);

    // FILE *fp;
    // uint32_t buffer[3];
    // uint64_t transf[2];
    // char fname[] = "file_test.txt";
    // fp = fopen(fname, "rb");
    // if (fp == NULL)
    //     {printf("Could not open file %s\n\a", fname); exit(42);}    
    // fread(buffer, sizeof(uint8_t), filesize, fp);
    // memcpy(transf, buffer, filesize);
    // return 0;

    // FILE *fp;
    // uint64_t buffer[0xff], bytes_read = 4; //initiate bytes_read to allow loop to run
    // char fname[] = "file_test.txt";
    // fp = fopen(fname, "rb");
    // if (fp == NULL)
    //     {printf("Could not open file %s\n\a", fname); exit(42);}    
    // for(uint64_t i = 1; bytes_read == 4; i += 2)
    //     {bytes_read = fread(((uint32_t *)buffer)+i, sizeof(uint8_t), 4, fp);}
    // return 0;

    FILE *fp;
    uint64_t buffer[64], bytes_read;
    char *fname = "file_test.txt";
    fp = fopen(fname, "rb");
    if (fp == NULL)
        {printf("Could not open file %s\n\a", fname); exit(42);}
    bytes_read = fread((uint8_t *)buffer, sizeof(uint8_t), 64, fp);
}