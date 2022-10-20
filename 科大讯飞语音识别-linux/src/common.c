#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>


unsigned long file_size_get(const char *pfile_path)
{
	unsigned long filesize = -1;	
	struct stat statbuff;
	
	if(stat(pfile_path, &statbuff) < 0)
	{
		return filesize;
	}
	else
	{
		filesize = statbuff.st_size;
	}
	
	return filesize;
}

unsigned char bcc_check(unsigned char *buf, int n)
{
    int i;
    unsigned char bcc=0;
	
    for(i = 0; i < n; i++)
    {
        bcc ^= *(buf+i);
    }
	
    return (~bcc);
}