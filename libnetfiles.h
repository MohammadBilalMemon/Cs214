#ifndef libnetfiles.h
#define libnetfiles.h




int netopen(const char *pathname, int flags);
ssize_t netread(int fildes, void *buf, size_t nbyte);
ssize_t netwrite(int fildes, const void *buf, size_t nbyte);
int netclose(int fd);
int netserverinit(char * hostname)




#endif
