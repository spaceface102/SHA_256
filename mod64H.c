#include <stdio.h>

int main(void)
{
	int x;
	printf("\n");
	for(int i = 0; i < 8; i++)
	{	for(int j = 0; j < 8; j++)
		{
			x = ((i+j)%8);
			printf("%d, ", x);
		}
		printf("\n");
	}
	return 0;
}
