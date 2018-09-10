
#include "stdio.h"
#include "randnum.h"
char bufr[128]={'\0'};
int goal=10;
int caicai=0;

PUBLIC void startGuessGame(int fd_stdin, int fd_stdout) {
    // Specify the rules of the game
    int i=0;
    printf("Welcome to NUM Game!Please enter a number!\n");   
    srandnum(1231);   
    goal= randnum() % 100;
  //  printf("value %d\n",goal);
  

   
while(1)
{ 
i=read(0,bufr,128);
caicai=getNum(bufr);
if(caicai==goal)
{printf("you win!\n");break;}
if(caicai>goal)
printf("bigger!\n");
if(caicai<goal)
printf("smaller!\n");
 }

}


int getNum(char*bufr)
{
int ten=1,i=0,res=0;
for(i=0;i<strlen(bufr)-1;i++)
ten*=10;
for(i=0;i<strlen(bufr);i++)
{
res+=(bufr[i]-'0')*ten;
ten/=10;
}
return res;

}       
   

