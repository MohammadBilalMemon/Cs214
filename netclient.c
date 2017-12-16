#include "libnetfiles.h"

int main()
{	
	char * readIn = (char*)malloc(sizeof(char) * 5000);
	char * readIn2 = (char*)malloc(sizeof(char) * 10);
	size_t amtToRead = 5000;
	size_t amt2Read = 30;
	size_t status = 0;
	size_t status2 = 0;
	char * buf = "grep.cs.rutgers.edu\n";


	networkserverinit("grep.cs.rutgers.edu");
	
	int fd3 = netopen("./notarealpath", O_RDONLY);
	netclose(fd3);
	int fd = netopen("./oz.txt", O_RDONLY);
	int fd2 = netopen("./test", O_RDWR);
	printf("fd %d\n", fd);
	printf("fd2 %d\n", fd2);

	status = netread(fd, readIn, (int)amtToRead);
	status2 = netread(fd2, readIn2, (int)amt2Read);

	printf("status: %d\n", status);
	printf("status2: %d\n", status2);
	printf("\n\n\n\n\n\n");
	printf("%s\n", readIn);
	printf("%s\n", readIn2);

	status = netwrite(fd2, buf, strlen(buf));
	printf("status 2: %d\n", status);
	printf("close fd: %d\n", netclose(fd));
	printf("close fd: %d\n", netclose(fd));
	printf("close fd2: %d\n", netclose(fd2));
	// printf("%d\n", netopen("./test2", O_RDONLY));
	// printf("%d\n", netopen("./testing", O_RDONLY));

	return 0;
}
