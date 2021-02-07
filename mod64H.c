#include <stdio.h>

int main(void)
{
	int x, j;
	printf("\n");
	for(int i = 0; i < 8; i++)
	{	printf("{");
		for(j = 0; j < 7; j++)
		{
			x = ((i+j)%8);
			printf("%d, ", x);
		}
		printf("%d},\n", ((i + j)%8));
	}
	return 0;
}
