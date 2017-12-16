#include "libnetfiles.h"

static const int portNum = 42942;

struct sockaddr_in serverAddressInfo;
const struct hostent *serverIPAddress;

int networkserverinit(char * hostname)
{
	// All connection and setup stuff
	/////////////////////////////////
	serverIPAddress = gethostbyname(hostname);
	
	// Error check to see if the host exists. I'll have to set h_errno at some point
	/////////////////////////////////
	if(serverIPAddress == NULL)
	{
		fprintf(stderr, "Can't find host\n");
		h_errno = HOST_NOT_FOUND;
		return -1;
	}


	bzero((char *) &serverAddressInfo, sizeof(serverAddressInfo));

	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_port = htons(portNum);

	bcopy((char *) serverIPAddress->h_addr, (char *)&serverAddressInfo.sin_addr.s_addr, serverIPAddress->h_length);

	// All good return 0 to say it worked.
	///////////////////////////////////
	return 0;

	// End of connection and setup stuff
	/////////////////////////////////////
	
}

int netopen(char * path, int mode)
{	
	// Set up for our message to send to server
	/////////////////////////////////
	char sendBuffer[256];
	int socketFD = getSockFD();
	int n;
	
	nLink * head = NULL;
	errno = 0;
	int err;
	int fd;


	// Error check for unsupported modes
	/////////////////////////////

	if(mode != O_RDONLY && mode != O_WRONLY && mode != O_RDWR)
	{
		fprintf(stderr, "Error: Trying to open file with a non-supported read/write mode.\n");
		n = write(socketFD, "Error,BadInput,", 16);	
		return -1;
	}

	// Construction of message to be sent. It'll need to be changed a bit
	///////////////////////////////////
	bzero(sendBuffer, 256);
	sprintf(sendBuffer, "open,%s,%d,", path, mode);

	// Send message
	///////////////////////////////////
	n = write(socketFD, sendBuffer, strlen(sendBuffer));

	// Error check
	//////////////////////////////////
	if(n < 0)
	{
		fprintf(stderr, "Couldn't write to socket.\n");
		return -1;
	}

	// Read from return socket
	////////////////////////////////
	bzero(sendBuffer,256);
	n = read(socketFD, sendBuffer, 255);

	// Parses the message from the socket and stores the data in a Linked list
	//////////////////////////////
	head = argPull(sendBuffer, head);

	// Error check of return socket
	////////////////////////////////
	if(n < 0)
	{
		fprintf(stderr, "Couldn't read from socket.\n");
		return -1;
	}

	// Checking on any errors from server side.
	////////////////////////////
	err = atoi(head->arg);
	errno = err;
	errNoChk(err);

	if(errNoChk(err) == 0)
	{
		return -1;
	}
	
	// Getting our actual return value and freeing some stuff
	////////////////////////////
	fd = atoi(head->next->arg);
	destroyList(head);

	return fd;
}

int netclose(int fd)
{
	// Set up for our message to send to server
	/////////////////////////////////
	char sendBuffer[256];
	int socketFD = getSockFD();
	int n;
	
	nLink * head = NULL;
	int result;
	int err;
	errno = 0;

	// Construction of message to be sent. It'll need to be changed a bit
	///////////////////////////////////
	bzero(sendBuffer, 256);
	sprintf(sendBuffer, "close,%d,", fd);

	// Send message
	///////////////////////////////////
	n = write(socketFD, sendBuffer, strlen(sendBuffer));

	// Error check
	//////////////////////////////////
	if(n < 0)
	{
		fprintf(stderr, "Couldn't write to socket.\n");
		return -1;
	}

	// Read from return socket
	////////////////////////////////
	bzero(sendBuffer,256);
	n = read(socketFD, sendBuffer, 255);

	// Error check of return socket
	////////////////////////////////
	if(n < 0)
	{
		fprintf(stderr, "Couldn't read from socket.\n");
		return -1;
	}

	// Parsing the message from the socket, setting errno and reporting errors
	//////////////////////////////
	head = argPull(sendBuffer, head);
	err = atoi(head->arg);
	errno = err;
	if(err == -1)
	{
		fprintf(stderr, "Couldn't close file.\n");
	}

	result = atoi(head->next->arg);

	return result;
}

ssize_t netread(int fd, void *buf, size_t nbyte)
{
	int n;
	int socketFD = getSockFD();
	if(fd == -1)
	{
		fprintf(stderr, "Error: Bad file descriptor\n");
		n = write(socketFD, "Error,BadInput", 5);
		return -1;
	}
	// Set up vars.
	/////////////////
	char sendBuffer[1000];
	char * recBuffer;

	errno = 0;
	int err;
	size_t bytesRead;
	char * readBuf;
	nLink * head = NULL;

	// Send message And error check
	///////////////////
	sprintf(sendBuffer, "read,%d,%d,", fd, nbyte);
	n = write(socketFD, sendBuffer, strlen(sendBuffer));

	if(n < 0)
	{
		fprintf(stderr, "Couldn't write to socket.\n");
		return -1;
	}

	// Read from return socket
	////////////////////////////////
	bzero(sendBuffer, 1000);

	n = read(socketFD, sendBuffer, 1000);

	// Parse return message and do some error checking
	////////////////////////////////
	head = readPull(sendBuffer, head);
	err = atoi(head->arg);
	
	if(errNoChk(err) == 0)
	{
		return -1;
	}

	// Making a new buffer of the proper size of the return message
	// then reading from socket stream into new buffer until we've gotten everything
	// out of the stream.
	///////////////////////////////
	bytesRead = atoi(head->next->arg);
	readBuf = malloc(sizeof(char) * bytesRead + 1);
	bzero(readBuf, bytesRead+1);
	sprintf(readBuf, "%s", head->next->next->arg);

	while(1)
	{
		if(strlen(readBuf) < bytesRead)
		{
			bzero(sendBuffer, 1000);
			n = read(socketFD, sendBuffer, 1000);
			sprintf(readBuf, "%s%s", readBuf, sendBuffer);
		}
		else
		{
			break;
		}
	}
	sprintf(buf, "%s%s", (char*)buf, readBuf);

	// Error check of return socket
	////////////////////////////////
	if(n < 0)
	{
		fprintf(stderr, "Couldn't read from socket.\n");
		free(readBuf);
		return -1;
	}	
	free(readBuf);
	return bytesRead;
}

ssize_t netwrite(int fd, const void *buf, size_t nbyte)
{
	// Bad fd check
	///////////////////////
	if(fd == -1)
	{
		fprintf(stderr, "Error: Bad file descriptor\n");
		return -1;
	}
	// Set up vars.
	/////////////////
	size_t size = intLen(fd) + strlen((char*)buf) + intLen(nbyte) + 6;
	char sendBuffer[size];
	int socketFD = getSockFD();
	int n;

	errno = 0;
	int err;
	size_t bytesWritten;
	char * readBuf;
	nLink * head = NULL;

	// Send message And error check
	///////////////////
	sprintf(sendBuffer, "write,%d,%d,%s", fd, nbyte, (char*)buf);
	n = write(socketFD, sendBuffer, strlen(sendBuffer));

	if(n < 0)
	{
		fprintf(stderr, "Couldn't write to socket.\n");
		return -1;
	}

	// Read from return socket
	////////////////////////////////
	bzero(sendBuffer,size);
	n = read(socketFD, sendBuffer, size);

	// Parse message, set errno, report errors
	/////////////////////////////////
	head = argPull(sendBuffer, head);
	err = atoi(head->arg);
	errno = err;

	if(errNoChk(err) == 0)
	{
		return -1;
	}

	// Get final data out and return it
	///////////////////////////////
	bytesWritten = atoi(head->next->arg);
	return bytesWritten;
}

// Gets a socket for the user to and returns a socket FD
////////////////////
int getSockFD()
{
	// Socket set up
	///////////////////////////////////
	int socketFD;

	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	
	// Error check for creating socket
	///////////////////////////////////
	if(socketFD < 0)
	{
		fprintf(stderr, "Couldn't make a socket.\n");
		return -1;
	}

	// Error check for connection
	//////////////////////////////////
	if(connect(socketFD, (struct sockaddr *)&serverAddressInfo, sizeof(serverAddressInfo)) < 0)
	{
		fprintf(stderr, "Couldn't connect to socket.\n");
		return -1;
	}

	return socketFD;
}

nLink * createLink(char * arg)
{
	// Used as a poor man's object constructor
	////////////////////////
	nLink * temp = (nLink*)malloc(sizeof(nLink));
	temp->arg = strdup(arg);
	temp->next = NULL;
	return temp;
}

nLink * addToLL(nLink * head, nLink * newnLink)
{
	// Hit the end of the list, simply add the node
	// Recursive.
	//////////////////////////
	if(head == NULL)
	{
		head = newnLink;
		return head;
	}

	else
	{
		head->next = addToLL(head->next, newnLink);
		return head;
	}
}

nLink * argPull(char * buffer, nLink * head)
{
	// Set up vars we'll need to pull strings from a buffer and 
	// add them to a linked list.
	//////////////////////////
	char * tempString;
	nLink * tempnLink;
	size_t startingPos = -1, endingPos = 0, sizeOfString = 0, len = 0, i = 0;
	len = strlen(buffer);
	for(i = 0; i <= len; i++)
	{
		// Check if current character is not a comma or a null term. 
		// makes some decisions based on that.
		///////////////////
		if(buffer[i] == ',' || buffer[i] == '\0')
		{
					// Nothing to do if current string is empty 
			///////////////////
			if(sizeOfString == 0)
			{
							continue;
			}
			// Grabs the current string from input and puts it into the list.
			///////////////////
			else
			{
							endingPos = i;
				tempString = pullString(startingPos, endingPos, sizeOfString, buffer);
				tempnLink = createLink(tempString);
				head = addToLL(head, tempnLink);
				free(tempString);
				startingPos = -1;
				sizeOfString = 0;	
			}
		}
		// Book keeping for current string.
		///////////////////
		else
		{
			if(startingPos == -1)
			{
				startingPos = i;
				sizeOfString++;
			}
			else
			{
				sizeOfString++;

			}
		}
	}

	return head;
}

nLink * readPull(char * buffer, nLink * head)
{
	// So I'm going to admit, this function and the next are bad.
	// I should have just be able to change argPull() to do what I wanted, but
	// I felt like I wanted very specific data out of these buffers and wanted
	// to do it this way.
	///////////////////
	char * tempString;
	nLink * tempnLink;
	size_t startingPos = 0, endingPos = 0, sizeOfString = 0, len = 0, i = 0;
	len = strlen(buffer);

	// Getting the command out
	///////////////
	for(i = 0; i < strlen(buffer); i++)
	{
		if(buffer[i] == ',')
		{
			endingPos = i;
			tempString = pullString(startingPos, endingPos, sizeOfString, buffer);
			tempnLink = createLink(tempString);
			head = addToLL(head, tempnLink);
			free(tempString);
			startingPos = endingPos + 1;
			sizeOfString = 0;
			break;
		}
		else
		{
			sizeOfString++;
		}
	}

	// Getting the FD out
	//////////////////
	for(i; i < strlen(buffer); i++)
	{
		if(buffer[i] == ',')
		{
			if(sizeOfString == 0)
			{
				continue;
			}
			else
			{
				endingPos = i;
				tempString = pullString(startingPos, endingPos, sizeOfString, buffer);
				tempnLink = createLink(tempString);
				head = addToLL(head, tempnLink);
				free(tempString);
				startingPos = endingPos + 1;
				sizeOfString = 0;
				break;
			}
		}
		else
		{
			sizeOfString++;
		}
	}

	// Getting the rest of the message
	/////////////////
	endingPos = strlen(buffer);
	tempString = pullString(startingPos, endingPos, strlen(buffer) - startingPos, buffer);
	tempnLink = createLink(tempString);
	head = addToLL(head, tempnLink);
	free(tempString);
	i += 2;
	startingPos = i;
	sizeOfString = 0;


	return head;
}

nLink * writePull(char * buffer, nLink * head)
{
	// Another bad function that's not the way I usually like to code.
	//////////////////////////
	char * tempString;
	nLink * tempnLink;
	size_t startingPos = 0, endingPos = 0, sizeOfString = 0, len = 0, i = 0;
	len = strlen(buffer);

	// Getting the command out
	///////////////
	for(i = 0; i < strlen(buffer); i++)
	{
		if(buffer[i] == ',')
		{
			endingPos = i;
			tempString = pullString(startingPos, endingPos, sizeOfString, buffer);
			tempnLink = createLink(tempString);
			head = addToLL(head, tempnLink);
			free(tempString);
			startingPos = endingPos + 1;
			sizeOfString = 0;
			break;
		}
		else
		{
			sizeOfString++;
		}
	}

	// Getting the FD out
	//////////////////
	for(i; i < strlen(buffer); i++)
	{
		if(buffer[i] == ',')
		{
			if(sizeOfString == 0)
			{
				continue;
			}
			else
			{
				endingPos = i;
				tempString = pullString(startingPos, endingPos, sizeOfString, buffer);
				tempnLink = createLink(tempString);
				head = addToLL(head, tempnLink);
				free(tempString);
				startingPos = endingPos + 1;
				sizeOfString = 0;
				break;
			}
		}
		else
		{
			sizeOfString++;
		}
	}

	// Getting the number of bytes to read out
	////////////////////////
	for(i; i < strlen(buffer); i++)
	{
		if(buffer[i] == ',')
		{
			if(sizeOfString == 0)
			{
				continue;
			}
			else
			{
				endingPos = i;
				tempString = pullString(startingPos, endingPos, sizeOfString, buffer);
				tempnLink = createLink(tempString);
				head = addToLL(head, tempnLink);
				free(tempString);
				startingPos = endingPos + 1;
				sizeOfString = 0;
				break;
			}
		}
		else
		{
			sizeOfString++;
		}
	}


	// Getting the rest of the message
	/////////////////
	endingPos = strlen(buffer);
	tempString = pullString(startingPos, endingPos, strlen(buffer) - startingPos, buffer);
	tempnLink = createLink(tempString);
	head = addToLL(head, tempnLink);
	free(tempString);
	i += 2;
	startingPos = i;
	sizeOfString = 0;

	return head;
}

// Helper method for my parsing methods
/////////////////
char * pullString(int start, int end, int size, char * originalString)
{
	int x, y;
	char * temp = (char*)calloc(size + 1, sizeof(char));
	for(x = 0, y = start; y < end; x++, y++)
	{
		temp[x] = originalString[y];
	}

	return temp;
}

// Helper method for my parsing methods
/////////////////
int intLen(int x)
{	
	int toReturn = 0;
	while(x > 0)
	{
		toReturn++;
		x /= 10;
	}

	return toReturn;
}

// Freeing some data from linked list
///////////////////////
void destroyList(nLink * head)
{
	if(head == NULL)
	{
		return;
	}
	else
	{
		destroyList(head->next);
		free(head->arg);
		free(head);
	}
}

// Error reporting and checking ugly and bad and I'm not even sure I should
// include it.
/////////////////////////
int errNoChk(int err)
{
	if(err == 0)
	{
		return 1;
	}
	else if(err == 1)
	{
		fprintf(stderr, "Error EPERM: Operation not permitted.\n");
	}
	else if(err == 2)
	{
		fprintf(stderr, "Error ENOENT: No such file.\n");
	}
	else if(err == 4)
	{
		fprintf(stderr, "Error EINTR: Interrupted system call.\n");
	}
	else if(err == 9)
	{
		fprintf(stderr, "Error EBADF: Bad file descriptor.\n");
	}
	else if(err == 13)
	{
		fprintf(stderr, "Error EACCES: You don't have permission to access that file.\n");
	}
	else if(err == 21)
	{
		fprintf(stderr, "Error EISDIR: Given path is a directory, not a file.\n");
	}
	else if(err == 23)
	{
		fprintf(stderr, "Error ENFILE: File table overflow.\n");
	}
	else if(err == 30)
	{
		fprintf(stderr, "Error EROFS: Read-only file system.\n" );
	}
	else if(err == 104)
	{
		fprintf(stderr, "Error ECONNRESET: Connection reset by peer.\n");
	}
	else if(err == 110)
	{
		fprintf(stderr, "Error ETIMEDOUT: Connection times out.\n");
	}

	return 0;
}
