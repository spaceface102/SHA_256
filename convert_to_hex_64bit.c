#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define max 6

int main(void)
{
	uint32_t H[8] = {
	1779033703, 3144134277, 1013904242, 2773480762, //current
	1359893119, 2600822924, 528734635, 1541459225};
   
    uint64_t H64bit[4];
    memcpy(H64bit, H, 4*sizeof(uint64_t));

    int i;
    printf("H[8] = {");
    for(i = 0; i < 7; i++)
    {
        if (!(i%max)) {printf("\n");}
        printf("0x%x, ", H[i]);
    }
    printf("0x%x}\n\n\n", H[i]);

    printf("H64bit[4] = {");
    for(i = 0; i < 3; i++)
    {
        if (!(i%(max/2))) {printf("\n");}
        printf("0x%lx, ", H64bit[i]);
    }
    printf("0x%lx}\n", H64bit[i]);

    printf("\nH[1] = %x\n", ((uint32_t *)H64bit)[1]);
}