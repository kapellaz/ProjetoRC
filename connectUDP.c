#include "Servidor.h"

void server_UDP(){
    child_pid = getpid();
    signal(SIGINT, SIG_IGN);
	SHM = shmat(id_shm,NULL,0);//memoria partilhada
	struct sockaddr_in server, admin_conec;
	int server_sck_desc, recv_len, send_len, saldo;
	socklen_t adlen = sizeof(admin_conec); 
	char buffer [BUFLEN];
	char *nome, *pass, *op, *permisao;
	int aproval = 0,i;
	
	// criar socket para a recepção dos pacotes UDP
	if((server_sck_desc=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1){
		erro("Erro na criação do socket");
	}
	
	//Preencher a informação do socket
	server.sin_family=AF_INET;
	server.sin_port = htons (PORTO_CONFIG);
	server.sin_addr.s_addr = inet_addr(IP_SERVER);
	
	if(bind(server_sck_desc,(struct sockaddr*)&server,sizeof(server))==-1){
		erro("Erro ao dar bind\n");
		exit(1);
	}
	while(1){//sistema de identificação do admin
		if((recv_len=recvfrom(server_sck_desc,buffer,BUFLEN,0,(struct sockaddr*)&admin_conec, &adlen)) ==-1){
				erro("Erro no recvfrom");
			}
			buffer[recv_len]='\0';//limpa o buffer
			nome=strtok(buffer," ");
			pass=strtok(NULL,"\n");
			if((strcmp(nome,admin.nome))==0 && strcmp(pass,admin.pass)==0){
				aproval=1;
				strcpy(buffer,"Login aceite\n");
				printf("%s",buffer);
				if((send_len = sendto(server_sck_desc,buffer,strlen(buffer),0,(struct sockaddr*)&admin_conec, adlen))==-1){
				erro("envio falhado\n");
				}
			}else{
				strcpy(buffer,"Login errado volte a tentar\n");
				printf("%s",buffer);
				if((send_len = sendto(server_sck_desc,buffer,strlen(buffer),0,(struct sockaddr*)&admin_conec, adlen))==-1){
				erro("envio falhado\n");
				}
			}
		
		while(aproval){//comunicação com o ADMIN
			if((recv_len=recvfrom(server_sck_desc,buffer,BUFLEN,0,(struct sockaddr*)&admin_conec, &adlen)) ==-1){
				erro("Erro no recvfrom");
			}
			buffer[recv_len]='\0';//limpa o buffer

			op=strtok(buffer,"\n ");
			printf("op: %s\n",op);
			
			if((strcmp(op,"ADD_USER"))==0){
				int encontrado=0;
				nome=strtok(NULL," ");
				pass=strtok(NULL," ");
				permisao=strtok(NULL," ");
				saldo=atof(strtok(NULL,"\n"));
				i=0;
				while(i<MAX_USERS){
					if(SHM->users[i].existe){
						if((strcmp(nome,SHM->users[i].nome))==0 && strcmp(pass,SHM->users[i].pass)==0){
							if(strcmp(SHM->mercado1.nome,permisao)==0){
								SHM->users[i].bolsas[0]=1;
							}
							if(strcmp(SHM->mercado2.nome,permisao)==0){
								SHM->users[i].bolsas[1]=1;
							}
							if(strcmp("BOTH",permisao)==0){
								SHM->users[i].bolsas[0]=1;
								SHM->users[i].bolsas[1]=1;
							}
							SHM->users[i].saldo=saldo;
							strcpy(buffer,"Utilizador atualizado\n");
							encontrado=1;
							break;
						}
					}
					i++;
				}
					if(!encontrado){//ainda existe espaço para users
						i=0;
						while(i<MAX_USERS){
							if(!SHM->users[i].existe){
								strcpy(SHM->users[i].nome,nome);
								strcpy(SHM->users[i].pass,pass);
								SHM->users[i].existe=true;
								if(strcmp(SHM->mercado1.nome,permisao)==0){
									SHM->users[i].bolsas[0]=1;
								}
								if(strcmp(SHM->mercado2.nome,permisao)==0){
									SHM->users[i].bolsas[1]=1;
								}
								if(strcmp("BOTH",permisao)==0){
									SHM->users[i].bolsas[0]=1;
									SHM->users[i].bolsas[1]=1;
								}
								SHM->users[i].saldo=saldo;
								strcpy(buffer,"Novo utilizador criado\n");

								encontrado=1;
								break;
							}
							i++;
						}
					}
					if (!encontrado){
						strcpy(buffer,"O limite de Users foi atingido\n");
					}
				printf("%s",buffer);
				if((send_len = sendto(server_sck_desc,buffer,strlen(buffer),0,(struct sockaddr*)&admin_conec, adlen))==-1){
				erro("envio falhado\n");
				}
			}
			else if((strcmp(op,"DEL"))==0){
				nome=strtok(NULL,"\n");
				i=0;
				while(i<MAX_USERS){
					if(strcmp(SHM->users[i].nome,nome)==0){
						SHM->users[i].existe=false;
						break;
					}
					i++;
				}
				strcpy(buffer,"Operacao realizada\n");
				printf("%s",buffer);
				if((send_len = sendto(server_sck_desc,buffer,strlen(buffer),0,(struct sockaddr*)&admin_conec, adlen))==-1){
				erro("envio falhado\n");
				}
			}
			else if((strcmp(op,"LIST"))==0){
				strcpy(buffer,"Iniciar a listagem\n");
				printf("%s",buffer);
				if((send_len = sendto(server_sck_desc,buffer,strlen(buffer),0,(struct sockaddr*)&admin_conec, adlen))==-1){
				erro("envio falhado\n");
				}
				i=0;
				while(i<MAX_USERS){
					if(SHM->users[i].existe){
						strcpy(buffer,SHM->users[i].nome);
						strcat(buffer,"\n");
						printf("%s",buffer);
						if((send_len = sendto(server_sck_desc,buffer,strlen(SHM->users[i].nome)+1,0,(struct sockaddr*)&admin_conec, adlen))==-1){
					erro("envio falhado\n");
						}
					}
					i++;
				}
					
				} 
			else if((strcmp(op,"REFRESH"))==0){
				SHM->REFRESHTIME=(int)atoi(strtok(NULL,"\n"));
				strcpy(buffer,"Refresh atualizado\n");
				printf("%s",buffer);
				if((send_len = sendto(server_sck_desc,buffer,strlen(buffer),0,(struct sockaddr*)&admin_conec, adlen))==-1){
				erro("envio falhado\n");
				}
			}
			else if((strcmp(op,"QUIT"))==0){
				aproval=0;
				strcpy(buffer,"Fecho da consola do administrador\n");
				printf("%s",buffer);
				if((send_len = sendto(server_sck_desc,buffer,strlen(buffer),0,(struct sockaddr*)&admin_conec, adlen))==-1){
				erro("envio falhado\n");
				}
				break;
			}
			else if((strcmp(op,"QUIT_SERVER"))==0){//inserir código para fechar o servidor
				strcpy(buffer,"Fecho do server\n");
				printf("%s",buffer);
				if((send_len = sendto(server_sck_desc,buffer,strlen(buffer),0,(struct sockaddr*)&admin_conec, adlen))==-1){
				erro("envio falhado\n");
				}
				close(server_sck_desc);
				kill(getppid(),SIGINT);
			}else{
				strcpy(buffer,"Comando errado\n");
				printf("%s",buffer);
				if((send_len = sendto(server_sck_desc,buffer,strlen(buffer),0,(struct sockaddr*)&admin_conec, adlen))==-1){
				erro("envio falhado\n");
				}
			}
		}
	}
}
