//gcc cliente.c -Wall -lpthread -D_REENTRANT -o clt
#include "Servidor.h"
void erro(char *msg);
void signal_handler(int signum);

int fd;
pthread_t *dados;

struct iniciar_dados{
	char ip[20];
	int  port;
};

void cleanarray(char array[]){
    for (int i = 0; i < BUFLEN; ++i) {
        array[i] = '\0';
    }
}

void *bolsa(void *void_dados){
	struct iniciar_dados *meus_dados=(struct iniciar_dados *) void_dados ;
	printf("inicio\n");
	struct sockaddr_in mercato;
	struct ip_mreq mreq;
	int len_mercato,fd_mercado,rcv_len;
	char msg[1024];
	fd_mercado = socket(AF_INET,SOCK_DGRAM,0);
	if(fd_mercado<0){
		perror("Erro no socket");
		exit(0);
	}
    int multicastTTL=255;
	if(setsockopt(fd_mercado,IPPROTO_IP,IP_MULTICAST_TTL,(void *) &multicastTTL, sizeof(multicastTTL))<0){
		perror("linking do socket");
		exit(0);
	}

	bzero((char *)&mercato,sizeof(mercato));
	mercato.sin_family = AF_INET;
	mercato.sin_addr.s_addr=htonl(INADDR_ANY);
	mercato.sin_port = htons(meus_dados->port);
	len_mercato=sizeof(mercato);
	if(bind(fd_mercado,(struct sockaddr *) &mercato, sizeof(mercato))<0){
		perror("Problemas no bind");
		exit(0);
	}
	mreq.imr_multiaddr.s_addr= inet_addr(meus_dados->ip);
	mreq.imr_interface.s_addr= htonl(INADDR_ANY);
	if(setsockopt(fd_mercado,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq))<0){
		perror("Problemas no setsocket");
		exit(0);
	}
	while(1){
		rcv_len=recvfrom(fd_mercado,msg,sizeof(msg),0,(struct sockaddr *) &mercato,(socklen_t *)&len_mercato);
		if(rcv_len < 0){
			perror("Problemas no recive");
			exit(0);
		}
		printf("%s", msg);
	}
}

int main(int argc, char *argv[]) {
	dados = (pthread_t *) malloc(sizeof (pthread_t) * 2);
	struct iniciar_dados dados1, dados2;

	pthread_join(dados[1],NULL);
    char endServer[100];

    struct sockaddr_in addr;
    struct hostent *hostPtr;
	
    if (argc != 3) {
        printf("cliente <host> <port>\n");
        exit(-1);
    }

    strcpy(endServer, argv[1]);
    if ((hostPtr = gethostbyname(endServer)) == 0)
        erro("Não consegui obter endereço");

    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
    addr.sin_port = htons((short) atoi(argv[2]));

    if ((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
        erro("socket");
    if (connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
        erro("Connect");

    char input[BUFLEN];
    char buffer[BUFLEN];

    cleanarray(input);

    int nread=0;

    int paraler=1;

    while (1){
        if(paraler==1) {
            nread = read(fd, buffer, BUFLEN - 1);
            while (nread <= 0) {
                nread = read(fd, buffer, BUFLEN - 1);
            }
        }
        buffer[nread-1]='\0';
        printf("%s", buffer);
        fscanf(stdin,"%s", input);
        paraler=1;
        if((strcmp(input,"DADOS")==0)){
            fscanf(stdin,"%s", input);
        	printf("IN\n");
        	if((strcmp(input, "ON")==0)){//pedir os endreços
        		write(fd, "BOLSAS", 7);
                nread = read(fd, buffer, BUFLEN-1);
                while(nread<=0){
                    nread = read(fd, buffer, BUFLEN-1);
                }
                printf("%s\n", buffer);
                char *aux = strtok(buffer, " ");
                if(strcmp(aux,"2")==0){
                    strcpy(dados1.ip,strtok(NULL, " "));
                    dados1.port=atoi(strtok(NULL, " "));
                    pthread_create(&dados[0],NULL,bolsa,&dados1);
                    strcpy(dados2.ip,strtok(NULL, " "));
                    dados2.port=atoi(strtok(NULL, "\n"));
                    pthread_create(&dados[1],NULL,bolsa,&dados2);
                }
                else if(strcmp(aux,"1")==0){
                    strcpy(dados1.ip,strtok(NULL, " "));
                    dados1.port=atoi(strtok(NULL, " "));
                    pthread_create(&dados[0],NULL,bolsa,&dados1);
                }else{
                    printf("Não tem acesso a nenhuma bolsa\n");
                }
        	}else if((strcmp(input, "OFF")==0)){
        	    pthread_cancel(dados[0]);
                pthread_cancel(dados[1]);
                paraler=0;
        	}
        }
        else {
            write(fd, input, strlen(input) + 1);
            if (strcmp(input, "SAIR") == 0 ||
                strcmp(buffer, "Varios utilizadores ligados!Tente novamente mais tarde") == 0) {
                close(fd);
                fclose(stdin);
                return 0;
            }
        }
        cleanarray(buffer);
        cleanarray(input);
    }
}

void erro(char *msg) {
    printf("Erro: %s\n", msg);
    exit(-1);
}

