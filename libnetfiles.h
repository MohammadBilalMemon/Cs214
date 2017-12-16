#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

typedef struct nLink
{
	char * arg;
	struct nLink * next;
}nLink;

int intLen(int x);
int getSockFD();
char * pullString(int start, int end, int size, char * originalString);
nLink * createLink(char * arg);
nLink * addToLL(nLink * head, nLink * newnLink);
nLink * argPull(char * buffer, nLink * head);
void destroyList(nLink * head);
int networkserverinit(char * hostname);
int netopen(char * path, int mode);
int nopen(nLink * head, int socketFD);
int netclose(int fd);
int nclose(nLink * head, int socketFD);
ssize_t netread(int fildes, void *buf, size_t nbyte);
int nread(nLink * head, int socketFD);
ssize_t netwrite(int fildes, const void *buf, size_t nbyte);
int nwrite(char * buffer, int socketFD);
nLink * readPull(char * buffer, nLink * head);
nLink * writePull(char * buffer, nLink * head);
void * threadMain(int * args);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int errNoChk(int err);
int pullSize(char * buffer);
