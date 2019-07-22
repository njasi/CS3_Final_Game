#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	struct ifaddrs *ifaddr;
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(1);
	}

	for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET) {
			continue;  
		}

		char host[NI_MAXHOST];
		getnameinfo(
			ifa->ifa_addr,
			sizeof(struct sockaddr_in),
			host, 
			NI_MAXHOST, 
			NULL, 
			0, 
			NI_NUMERICHOST
		);

		printf("%s: %s\n", ifa->ifa_name, host);
	}

	freeifaddrs(ifaddr);
	return 0;
}
