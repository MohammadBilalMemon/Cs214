#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[] )
{

//variables

int sock; //gives return value of socket function
struct sockaddress_in server;
int mysock; // to hold client connection
char buff[1024]; // to hold data
int rval; // return value





// create socket

sock = socket(AF_INET, SOCK_STREAM, 0);
if (sock < 0)
{
perror("unable to create socket");
exit(0);
}
server.sin_family = AF_INET;
server.sin_addr.s_addr = INADDR_ANY;
server.sin_port = 5000;





// call bind
 if(bind(sock, (struct sockaddr *)&server, sizeof(server)))
 {
  perror("Bind failed.");
  exit(1);
 }




// listen to network



// accept








return 0;
}

//"14.netread -99/.100"
//read until 1st comma... take those chars and atoi them.... this is the remaining message length
//read until next comma... those chars are the command...
//(in this case I know I need to do a read)

case NETREAD
//read until next comma. know these chars are net file descriptors.. look up netFD in a sate table, save local fd as parameterA
//read until end of bytes. Know those chars are amount to read. atoi to int. save as parameterB
//read(parameterA, buffer, parameterB) //run local command with remote parameters
