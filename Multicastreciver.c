#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/shm.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BOLSA1 "239.0.0.1"
#define PORTO1  7777

int main(){
	struct sockaddr_in mercato_1;
	struct ip_mreq mreq;
	int len_mercato_1,fd_mercado1,rcv_len;
	char msg[1024];
fd_mercado1 = socket(AF_INET,SOCK_DGRAM,0);
	if(fd_mercado1<0){
		perror("Erro no socket");
		exit(0);
	}
int multicastTTL=255;
	if(setsockopt(fd_mercado1,IPPROTO_IP,IP_MULTICAST_TTL,(void *) &multicastTTL, sizeof(multicastTTL))<0){
		perror("linking do socket");
		exit(0);
	}
	bzero((char *)&mercato_1,sizeof(mercato_1));
	mercato_1.sin_family = AF_INET;
	mercato_1.sin_addr.s_addr=htonl(INADDR_ANY);
	mercato_1.sin_port = htons(PORTO);
	len_mercato_1=sizeof(mercato_1);
	if(bind(fd_mercado1,(struct sockaddr *) &mercato_1, sizeof(mercato_1))<0){
		perror("Problemas no bind");
		exit(0);
	}
	mreq.imr_multiaddr.s_addr= inet_addr(BOLSA1);
	mreq.imr_interface.s_addr= htonl(INADDR_ANY);
	if(setsockopt(fd_mercado1,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))<0){
		perror("Problemas no setsocket");
		exit(0);
	}
	while(1){
		rcv_len=recvfrom(fd_mercado1,msg,sizeof(msg),0,(struct sockaddr *) &mercato_1,(socklen_t *)&len_mercato_1);
		if(rcv_len < 0){
			perror("Problemas no recive");
			exit(0);
		}
		printf("%s", msg);
	}
}
