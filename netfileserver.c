#include "libnetfiles.h"

// I have no clue how to use mutexes so I end up 
// just using one for everything. Terrible solution,
// but it's all I could get working.
///////////////////////////////
pthread_mutex_t mutex;

int main()
{
	// Set up vars to use in our program
	//////////////////////////////////
	socklen_t client;
	int socketFD;
	int * newSocketFD = malloc(sizeof(int));
	int portNum = 42942;
	pthread_t netpthread;

	// Setting up our connection
	////////////////////////////////
	struct sockaddr_in serverAddressInfo;
	struct sockaddr_in clientAddressInfo;

	socketFD = socket(AF_INET, SOCK_STREAM, 0);

	// Error checks for some connection stuff
	////////////////////////////////
	if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0)
	{
	    fprintf(stderr, "setsockopt(SO_REUSEADDR) failed");
	}

	if(socketFD < 0)
	{
		fprintf(stderr, "Can't open FD\n");
		return -1;
	}

	bzero((char *) &serverAddressInfo, sizeof(serverAddressInfo));
	serverAddressInfo.sin_port = htons(portNum);
	serverAddressInfo.sin_family = AF_INET;
	serverAddressInfo.sin_addr.s_addr = INADDR_ANY;

	if(bind(socketFD, (struct sockaddr *) &serverAddressInfo, sizeof(serverAddressInfo)) < 0)
	{
		fprintf(stderr, "Can't bind socket\n");
		return -1;
	}

	// End of connection set up
	////////////////////////////////


	// Main loop of code. Siting listening and make threads
	//////////////////////////////
	while(1)
	{
		listen(socketFD, 5);

		client = sizeof(clientAddressInfo);
		*newSocketFD = accept(socketFD, (struct sockaddr *) &clientAddressInfo, &client);

		if(*newSocketFD < 0)
		{
			fprintf(stderr, "Couldn't accept connection\n");
		}

		pthread_create(&netpthread, NULL, (void*)threadMain, newSocketFD);
		pthread_detach(netpthread);
	}
	
	return 0;
}

// This takes in the socket FD parses some of the message
// see which function needs to be run and then call those 
// from threadMain.
////////////////////////
void * threadMain(int * args)
{
	// Set up of vars
	///////////////////////
	int n;
	int newSocketFD = (int)*args;
	char buffer[256];
	bzero(buffer, 256);
	n = read(newSocketFD, buffer, 255);
	if(n < 0)
	{
		fprintf(stderr, "Couldn't read from socket\n");
		pthread_exit(NULL);
	}

	// Parse incoming packet
	//////////////////////
	nLink * head = NULL;
	head = argPull(buffer, head);

	nLink * temp = head;
	char * cmd = temp->arg;
	// Checking which function to call
	//////////////////////
	if(strncmp("open", cmd, 4) == 0)
	{
		nopen(head, newSocketFD);
	}
	else if(strncmp("read", cmd, 4) == 0)
	{
		nread(head, newSocketFD);
	}
	else if(strncmp("close", cmd, 5) == 0)
	{
		nclose(head, newSocketFD);
	}
	else if(strncmp("write", cmd, 5) == 0)
	{
		destroyList(head);
		nwrite(buffer, newSocketFD);
	}

	// Not one of the above, just write back and error
	///////////////////////
	n = write(newSocketFD, "Error: Can't parse incoming packet.", 36);
	if(n < 0)
	{
		fprintf(stderr, "Couldn't write to socket.\n");
		pthread_exit(NULL);
	}

	pthread_exit(NULL);
}

// Function that opens a file and sends a fd packet
//////////////////////
int nopen(nLink * head, int socketFD)
{
	// Var set up 
	//////////////////
	int msgSize;
	int err;
	int n;
	int mode;
	int newFD;
	char * path;
	char * message;

	// Pulling out of arguments from linked list
	/////////////////
	nLink * tmp = head;

	// Skip the command, we don't need it
	/////////////////
	tmp = tmp->next;

	// Pulling out path argument
	///////////////////
	path = tmp->arg;

	// Pulling out and atoi() the mode arg
	///////////////////
	tmp = tmp->next;
	mode = atoi(tmp->arg);
	
	// Actually opening the file and error check
	/////////////////
	newFD = open(path, mode);
	err = errno;
	errno = 0;
	if(newFD != -1)
	{
		newFD *= -1;
	}
	
	// Setting errno and getting my message ready to send
	////////////////
	msgSize = intLen(err) + intLen(newFD);

	message = (char*)malloc(sizeof(char) * msgSize + 1);
	sprintf(message, "%d,%d,", err, newFD);
	
	// Writing new socket and error check
	////////////////
	n = write(socketFD, message, strlen(message));

	if(n < 0)
	{
		fprintf(stderr, "Couldn't write to socket.\n");
		return -1;
	}

	destroyList(head);
	return 0;
}

// Function that closes a file and sends a status packet
//////////////////////
int nclose(nLink * head, int socketFD)
{
	// Var set up
	//////////////
	int n;
	int err;
	int result;
	int intFD = atoi(head->next->arg);
	char * message;
	// Change to local FD
	//////////////
	if(intFD != -1)
	{
		intFD *= -1;
	}
	
	// Actually closing and checking errno
	//////////////
	result = close(intFD);
	err = errno;
	errno = 0;
	
	// getting the proper size of the message and putting status into it/
	////////////////
	message = (char*)malloc(sizeof(char) * (intLen(result) + intLen(err) + 1));
	sprintf(message, "%d,%d,", err, result);
	
	n = write(socketFD, message, strlen(message) + 1);

	if(n < 0)
	{
		fprintf(stderr, "Couldn't write to socket.\n");
		return -1;
	}

	destroyList(head);
	return result;
}

// Function that reads from a file and sends a content packet
//////////////////////
int nread(nLink * head, int socketFD)
{
	// Var set up
	////////////////
	int n;
	int err;
	char * message;

	// Getting proper FD
	////////////////	
	int intFD = atoi(head->next->arg);

	if(intFD != -1)
	{
		intFD *= -1;
	}

	// I know this is lazy, but it's getting late.
	/////////////////
	size_t intSize = atoi(head->next->next->arg);
	int status;
	// Reading the file
	/////////////////
	char * buffer = (char*)malloc( sizeof(char) * intSize + 1);

	pthread_mutex_lock(&mutex);
	status = read(intFD, buffer, intSize);
	pthread_mutex_unlock(&mutex);
	// Pulling out an error info from last call
	/////////////////
	err = errno;
	errno = 0;
	if(status < 0)
	{
		fprintf(stderr, "Error reading from file\n");
		return -1;
	}
	// Assembling the return packet.
	////////////////////
	message = (char*)malloc(sizeof(char) * (strlen(buffer) + intLen(status) + intLen(err) + 1) );
	sprintf(message, "%d,%d,%s", err, status, buffer);
	n = write(socketFD, message, strlen(message));

	if(n < 0)
	{
		fprintf(stderr, "Couldn't write to socket.\n");
		return -1;
	}
	return 0;
}

int nwrite(char * buffer, int socketFD)
{
	// Var set up
	////////////////
	nLink * head = NULL;
	head = writePull(buffer, head);
	nLink * temp = head;
	temp = temp->next;
	int n;
	int err;
	int status;
	int intFD;
	size_t intSize;
	char * writeBuffer;
	char * message;

	// Getting proper FD 
	////////////////	
	intFD = atoi(temp->arg);
	if(intFD != -1)
	{
		intFD *= -1;
	}

	// Getting and atoi number of bytes to read
	////////////////////////
	temp = temp->next;
	intSize = atoi(temp->arg);

	// Getting out and chars to write
	////////////////////////
	temp = temp->next;
	buffer = temp->arg;

	// Assembling the buffer that will be written to a file.
	/////////////////////////
	writeBuffer = (char*)malloc(sizeof(char) * intSize + 1);
	sprintf(writeBuffer, "%s%s", writeBuffer, buffer);

	// Checking the socket stream for any info that wasn't pulled out.
	//////////////////////
	while(1)
	{
		if(strlen(writeBuffer) < intSize)
		{
			bzero(buffer, 256);
			n = read(socketFD, buffer, 255);
			sprintf(writeBuffer, "%s%s", writeBuffer, buffer);
		}
		else
		{
			break;
		}
	}


	// Reading the file
	/////////////////	
	pthread_mutex_lock(&mutex);
	status = write(intFD, writeBuffer, intSize);
	pthread_mutex_unlock(&mutex);

	// Some error checking stuff
	///////////////
	err = errno;
	errno = 0;

	if(status < 0)
	{
		fprintf(stderr, "Error writing to file\n");
		return -1;
	}

	// Assembling return packet.
	//////////////////
	message = (char*)malloc(sizeof(char) * intLen(status) + intLen(err));
	sprintf(message, "%d,%d,", err, status);

	n = write(socketFD, message, strlen(message));

	if(n < 0)
	{
		fprintf(stderr, "Couldn't write to socket.\n");
		return -1;
	}
	destroyList(head);

	return 0;
}
