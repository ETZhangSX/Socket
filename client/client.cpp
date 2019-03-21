#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
	struct hostent *ht = NULL;

	char host[] = "www.etzhang.xyz";

	ht = gethostbyname(host);
	printf("Host name: %s\n", ht->h_name);
	printf("IP: \n");
	int i = 0;
	while (ht->h_addr_list[i] != NULL)
		printf("%s", (ht->h_addr_list[i]));
	
	printf("\n");
}
