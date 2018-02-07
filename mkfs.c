#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>

int main(){
int i;
int buf[10]={0,0,0,0,0,0,0,0,0,0};
int fd = open("file.txt",O_CREAT|O_RDWR,0666);
for(i=0;i<(1024*1024*1024/10);i++)
	write(fd,(void*)buf,sizeof(buf));
return 0;
}
