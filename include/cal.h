#pragma once
#include "stdio.h"
PUBLIC void startcal(int fd_stdin, int fd_stdout)
{
	int op = 0;	
	char bu[128] = { '\0' };
char bu2[128] = { '\0' };	
	int result;	
	int i = 0;
	int firstnum=0;
	int secondnum = 0;
	printf("--------------------\n");
	
	printf("        1.+       \n");
	printf("        2.-       \n");
	printf("        3.*       \n");
	printf("        4./       \n");
        printf("    Input your choice \n");
	printf("--------------------\n");
	i = read(0, bu, 128);
	op = getNum(bu);	
	printf("Please input the first operting number:\n");
        read(0, bu2, 128);	
	firstnum = getNum(bu2);
	printf("Please input the second operting number:\n");
	i = read(0, bu, 128);
	secondnum = getNum(bu);
	switch (op)
	{
	case 1:
		printf("%d+%d=%d\n", firstnum, secondnum, result = firstnum + secondnum);   //实现加法功能
		break;
	case 2:
		printf("%d-%d=%d\n", firstnum, secondnum, result = firstnum - secondnum);     //实现减法功能
		break;
	case 3:
		printf("%d*%d=%d\n", firstnum, secondnum, result = firstnum * secondnum);      // 实现乘法功能
		break;
	case 4:
		printf("%d-%d=%d\n", firstnum, secondnum, result = firstnum / secondnum);      //实现除法功能
		break;
	default:
		printf("error");
		break;
	}
		
		
		
}
