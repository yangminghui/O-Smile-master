
#include "stdio.h"

int q[20];//各皇后所在的行号
int amount = 0;
int n;
void printqueen(int n)
{
	int i, j;
	amount++;
	printf("\n");
	for (i = 1; i <= n; ++i)
	{
		for (int j = 1; j <= n; ++j)
		{
			if (q[i] != j)
			{
				printf("0 ");
			}
			else
				printf("X ");
		}

		printf("\n");
	}

}

int find(int i, int k)
{
	int j = 1;
	while (j < i)
	{
		if (q[j] == k || abs(j - i) == abs(q[j] - k))
			return 0;
		j++;
	}
	return 1;
}
void place(int k, int n)
{
	int j;
	if (k > n)
		printqueen(n);
	else
	{
		for (j = 1; j <= n; j++)
		{
			if (find(k, j))
			{
				q[k] = j;
				place(k + 1, n);
			}
		}
	}
}
PUBLIC void startqueen(int fd_stdin, int fd_stdout)
{
	
        int i;char buqe[128]={'\0'};
	printf("please input the value\n");

	//printf( "input\n");
        i=read(0,buqe,128);
        n=getNum(buqe);
      //  printf("n is %d\n", n);
	place(1, n);
	printf("result is %d\n", amount);

	return 0;
}

int abs(int a)
{
if(a>=0)
return a;
if(a<0)
return -a;
}


















\
