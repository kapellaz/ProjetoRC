/*
* Servidor espera por conecções UDP no porto 9000. 
* Ip= 
*/

#include "Servidor.h"

struct sockaddr_in mercato_1, mercato_2;
int len_mercato_1,len_mercato_2;
int fd_mercado1, fd_mercado2;
void erro(char *s){
	perror(s);
	exit(1);
}


void lefichconfig(FILE *f){
	int i;
	char linha[250];
	fscanf(f,"%s",linha);
	strcpy(admin.nome,strtok(linha,"/"));
	strcpy(admin.pass,strtok(NULL,"\n"));
	fscanf(f,"%s",linha);
	int n_users=atoi(strtok(linha,"\n"));
	if(n_users>5){
		erro("numero incorreto de users");
	}
	for(i=0;i<n_users;i++){
	    fscanf(f,"%s",linha);
		SHM->users[i].existe=true;
		strcpy(SHM->users[i].nome,strtok(linha,";"));
		strcpy(SHM->users[i].pass,strtok(NULL,";"));
        for (int j = 0; j < 6; ++j) {
            SHM->users[i].acoes[j]=0;
        }
		SHM->users[i].saldo=atof(strtok(NULL,"\n"));
        SHM->users[i].bolsas[0]=0;
        SHM->users[i].bolsas[1]=0;
	}
	fscanf(f,"%s",linha);
	strcpy(SHM->mercado1.nome,strtok(linha,";"));
	strcpy(SHM->mercado1.acoes[0].nome,strtok(NULL,";"));
	SHM->mercado1.acoes[0].valorcompra=atof(strtok(NULL,"\n"));
    SHM->mercado1.acoes[0].valorvenda=SHM->mercado1.acoes[0].valorcompra+0.02;
    SHM->mercado1.acoes[0].index=0;
    SHM->mercado1.acoes[0].quantidade=rand()%10*100;
	i=1;
	acoes_b1=1;
	acoes_b2=0;
    srand(time(NULL));
	while(fscanf(f,"%s",linha)!=EOF){
		if(strcmp(SHM->mercado1.nome,strtok(linha,";"))==0){
			strcpy(SHM->mercado1.acoes[acoes_b1].nome,strtok(NULL,";"));
			SHM->mercado1.acoes[acoes_b1].index=acoes_b1;
			SHM->mercado1.acoes[acoes_b1].valorcompra=atof(strtok(NULL,"\n"));
            SHM->mercado1.acoes[acoes_b1].valorvenda=SHM->mercado1.acoes[acoes_b1].valorcompra+0.02;
            SHM->mercado1.acoes[acoes_b1].quantidade=rand()%10*100;
			acoes_b1++;
		}else{
			strcpy(SHM->mercado2.nome,linha);
            SHM->mercado2.acoes[acoes_b2].index=2+acoes_b2;
            SHM->mercado2.acoes[acoes_b2].quantidade=rand()%10*100;
			strcpy(SHM->mercado2.acoes[acoes_b2].nome,strtok(NULL,";"));
            SHM->mercado2.acoes[acoes_b2].valorcompra=atof(strtok(NULL,"\n"));
            SHM->mercado2.acoes[acoes_b2].valorvenda=SHM->mercado1.acoes[acoes_b2].valorcompra+0.02;
			acoes_b2++;
		}
	}
}



void *timer_bolsa(void *id){
	char mensagem1[1024], mensagem2[1024], aux[200];
	int random, i;
	mercato_1.sin_addr.s_addr=inet_addr(BOLSA1);
	mercato_2.sin_addr.s_addr=inet_addr(BOLSA2);
    while(1){
    	sprintf(mensagem1,"Dados das ações da bolsa 1\n");
        for(i=0;i<acoes_b1;i++){
            random=rand()%2;
            if((random==1 || SHM->mercado1.acoes[i].valorvenda==0.1)||SHM->mercado1.acoes[i].valorcompra==0.1){
                SHM->mercado1.acoes[i].valorvenda=SHM->mercado1.acoes[i].valorvenda+0.1;
                SHM->mercado1.acoes[i].valorcompra=SHM->mercado1.acoes[i].valorcompra+0.1;
            }else{
                SHM->mercado1.acoes[i].valorvenda=SHM->mercado1.acoes[i].valorvenda-0.1;
                SHM->mercado1.acoes[i].valorcompra=SHM->mercado1.acoes[i].valorcompra-0.1;
            }
            sprintf(aux,"A acao %s está com um valor de venda de %f e numa quantidade de %d\n",SHM->mercado1.acoes[i].nome,SHM->mercado1.acoes[i].valorvenda,SHM->mercado1.acoes[i].quantidade);
            strcat(mensagem1,aux);
        }
        sprintf(mensagem2,"Dados das ações da bolsa 2\n");
        for(i=0;i<acoes_b2;i++){
            random=rand()%2;
            if((random==1 || SHM->mercado2.acoes[i].valorvenda==0.1) ||SHM->mercado2.acoes[i].valorcompra==0.1){
                SHM->mercado2.acoes[i].valorvenda=SHM->mercado2.acoes[i].valorvenda+0.1;
                SHM->mercado2.acoes[i].valorcompra=SHM->mercado2.acoes[i].valorcompra+0.1;
            }else{
                SHM->mercado2.acoes[i].valorvenda=SHM->mercado2.acoes[i].valorvenda-0.1;
                SHM->mercado2.acoes[i].valorcompra=SHM->mercado2.acoes[i].valorcompra-0.1;
            }
            sprintf(aux,"A acao %s está com um valor de venda de %f e numa quantidade de %d\n",SHM->mercado2.acoes[i].nome,SHM->mercado2.acoes[i].valorvenda,SHM->mercado2.acoes[i].quantidade);
            strcat(mensagem2,aux);
        }
        printf("Vou enviar\n");
        sendto(fd_mercado1,mensagem1,sizeof(mensagem1),0,(struct sockaddr *) &mercato_1, len_mercato_1);
        sendto(fd_mercado2,mensagem2,sizeof(mensagem2),0,(struct sockaddr *) &mercato_2, len_mercato_2);
        printf("Envio feito pippippip\n");
        sleep(SHM->REFRESHTIME);
	}
}

void sighandler(int num){
    kill(tcp_pid, SIGTERM);
    kill(child_pid, SIGTERM);
    while(wait(NULL)!=-1);
    shmdt(SHM);
    shmctl(id_shm,IPC_RMID,NULL);
    kill(pidmain, SIGTERM);
}


int main(int argc, char *argv[]){//trata de receber as ligações TCP e de as reencaminhar inicia também o processo UDP
	if (argc != 4) {
    printf("cliente <Porto_Bolsa> <Porto_Config> <Fich_Config>\n");
    exit(-1);
    }


	//Cria a memoria partilhada
    pidmain=getpid();
    signal(SIGINT, sighandler);
    pthread_t *timer;


	timer = (pthread_t *) malloc(sizeof (pthread_t));

	if((id_shm=shmget(IPC_PRIVATE,sizeof(struct bloco),IPC_CREAT|0777))<0){
		erro("Criação da shm");
	}
	printf("%s --- %s \n",argv[1],argv[2]);
	PORTO_BOLSA = atoi(argv[1]);
	PORTO_CONFIG = atoi(argv[2]);
	SHM = shmat(id_shm,NULL,0);
	SHM->REFRESHTIME=2;
	FILE *f_config=fopen(argv[3],"r");
	lefichconfig(f_config);

	//Cria o socket de multicast para cada bolsa
	fd_mercado1 = socket(AF_INET,SOCK_DGRAM,0);
	if(fd_mercado1<0){
		erro("Erro no socket");
	}
	int multicastTTL=255;
	if(setsockopt(fd_mercado1,IPPROTO_IP,IP_MULTICAST_TTL,(void *) &multicastTTL, sizeof(multicastTTL))<0){
		erro("linking do socket");
	}
	bzero((char *)&mercato_1,sizeof(mercato_1));
	mercato_1.sin_family = AF_INET;
	mercato_1.sin_addr.s_addr=htonl(INADDR_ANY);
	mercato_1.sin_port = htons(PORTO_MULTICAST1);
	len_mercato_1=sizeof(mercato_1);

	fd_mercado2 = socket(AF_INET,SOCK_DGRAM,0);
	if(fd_mercado2<0){
		erro("Erro no socket");
	}
	if(setsockopt(fd_mercado2,IPPROTO_IP,IP_MULTICAST_TTL,(void *) &multicastTTL, sizeof(multicastTTL))<0){
		erro("linking do socket");
	}
	bzero((char *)&mercato_2,sizeof(mercato_2));
	mercato_2.sin_family = AF_INET;
	mercato_2.sin_addr.s_addr=htonl(INADDR_ANY);
	mercato_2.sin_port = htons(PORTO_MULTICAST2);
	len_mercato_2=sizeof(mercato_2);
	//
	pthread_create(timer,NULL,timer_bolsa,NULL);

	//tratamento das chamadas UDP
    int pid=fork();
     if(pid==0){
        server_UDP();
        exit(0);
     }
	// tratamento das chamadas TCP
	int pid2=fork();
 	if(pid2==0){
 		server_TCP();
 		exit(0);
 	}
	//clean up
	pthread_join(timer[0],NULL);
	wait(NULL);
	wait(NULL);
	exit(0);
}
