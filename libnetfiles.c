#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>  //open() 
#include <sys/stat.h>  //open()
#include <fcntl.h> //open()
#include <pthread.h> 
#include <sys/socket.h>
#include <netdb.h>
#include "libnetfiles.h"

int netserverinit(const char * hostname){
	int port; // Greater than 8k and less than 64k
	int server = gethostbyname(hostname); //http://beej.us/guide/bgnet/output/html/multipage/gethostbynameman.html
	if (server == NULL)
	{
		fprintf(stderr,"HOST_NOT_FOUND");
		return -1;
		
		
	}
	
	int netopen(const char *pathname, int flags)
	{
		if(flags ==  O_RDONLY)
	{
		check = O_RDONLY;		
	}
	else if( flags == O_WRONLY )
	{
		check = O_WRONLY;
	}
	else if(flags ==  O_RDWR )
	{
		check =  O_RDWR;
	}
		
		
	}
	
	EACCES // permission denied
	EINTR // interrupted function callo
	
	
	
	
	
	
	}
	





int netopen(const char *pathname, int flags) {
	
	int check  = 0;
	
	
	
	

	
	
	
	
}