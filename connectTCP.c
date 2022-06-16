#include "Servidor.h"


int verifica_user(char nome[], char pass[]){
    int permissao=0;
    for (int i = 0; i <MAX_USERS; i++) {
        if(strcmp(nome, SHM->users[i].nome)==0 && strcmp(pass, SHM->users[i].pass)==0){
            for (int j = 0; j < 2; ++j) {
                if(SHM->users[i].bolsas[j]==1){
                    permissao=permissao+j+1;
                }
            }
            return permissao;
        }
    }
    return -1;
}

void cleanarray(char array[]){
    for (int i = 0; i < BUFLEN; ++i) {
        array[i] = '\0';
    }
}


int findacao(char nomeacao[]) {
    for (int i = 0; i < 6; ++i) {
        if (i < 3) {
            if (strcmp(SHM->mercado1.acoes[i].nome, nomeacao) == 0) {
                return SHM->mercado1.acoes[i].index;
            }
        } else {
            if (strcmp(SHM->mercado2.acoes[i].nome, nomeacao) == 0) {
                return SHM->mercado2.acoes[i].index+3;
            }
        }
    }
    return -1;
}


void enviaaddr(int client, char user[]){
printf("Pedido de dados recebido\n");
struct user atual;
char buffer[BUFLEN];
	for (int i = 0; i < MAX_USERS; ++i) {//encontra o user
        if(strcmp(SHM->users[i].nome, user)==0){
        	atual=SHM->users[i];
        	break;
        }
    }
    if(atual.bolsas[0]==1 && atual.bolsas[1]==1){
    	sprintf(buffer,"2 %s %d %s %d\n",BOLSA1,PORTO_MULTICAST1,BOLSA2,PORTO_MULTICAST2);
    }else if(atual.bolsas[0]==1){
    	sprintf(buffer,"1 %s %d\n",BOLSA1,PORTO_MULTICAST1);
    }else if(atual.bolsas[1]==1){
    	sprintf(buffer,"1 %s %d\n",BOLSA2,PORTO_MULTICAST2);
    }else{
    	sprintf(buffer,"0 Não tem acesso a nenhuma bolsa\n");
    }
    write(client,buffer,BUFLEN-1);
    write(client,"Impressao dos dados ativada!!!",BUFLEN-1);
}


void compra(int client, char user[]){
    int indexacao;
    char buffer[BUFLEN];
    char nomeacao[100];
    int quant;
    int preco;
    int indice_cliente;
    cleanarray(buffer);
    sprintf(buffer, "Insira o nome e numero de ações que quer comprar e o preco que deseja pagar:\n"
                    "(formato acao-numerodeacoes-preco)\n");
    write(client, buffer, BUFLEN-1);
    int nread = read(client, buffer, BUFLEN-1);
    while(nread<=0){
        nread = read(client, buffer, BUFLEN-1);
    }
    buffer[nread - 1] = '\0';
    strcpy(nomeacao, strtok(buffer, "-"));
    indexacao=findacao(nomeacao);
    if(indexacao>=0){
        quant= atoi(strtok(NULL, "-"));
        if(quant%10!=0&&quant<101){
            cleanarray(buffer);
            sprintf(buffer, "A quantidade que deseja comprar deve ser um multiplo de 10 e no maximo 100\n");
            write(client, buffer, BUFLEN-1);
        }
        preco= atoi(strtok(NULL, "\n"));
        printf("%s, %d, %d", nomeacao, quant, preco);
    }else{
        cleanarray(buffer);
        sprintf(buffer, "Esta acao nao existe\n");
        write(client, buffer, BUFLEN-1);
        return;
    }
    cleanarray(buffer);
    sprintf(buffer, "Deseja proceder com a operação?(S/N)\n");
    write(client, buffer, BUFLEN-1);
    cleanarray(buffer);
    nread = read(client, buffer, BUFLEN-1);
    while(nread<=0){
        nread = read(client, buffer, BUFLEN-1);
    }
    buffer[nread-1]='\0';
    if(strcmp(buffer, "S")==0){
        printf("%s, %d, %d", nomeacao, quant, preco);
        for (int i = 0; i < MAX_USERS; ++i) {
            if(strcmp(SHM->users[i].nome, user)==0){
                indice_cliente=i;
                if(preco*quant>SHM->users[i].saldo){
                    cleanarray(buffer);
                    sprintf(buffer, "Não tem saldo suficiente\n");
                    write(client, buffer, BUFLEN-1);
                    return;
                }else{
                    if(indexacao<3 && preco < SHM->mercado1.acoes[indexacao].valorvenda) {
                        cleanarray(buffer);
                        sprintf(buffer, "Nao existem acoes pelo preco que deseja pagar\n");
                        write(client, buffer, BUFLEN - 1);
                        return;
                    }
                    else{
                        if(preco<SHM->mercado1.acoes[indexacao-3].valorvenda){
                            cleanarray(buffer);
                            sprintf(buffer, "Nao existem acoes pelo preco que deseja pagar\n");
                            write(client, buffer, BUFLEN-1);
                            return;
                        }
                    }
                }
                int quantidadedisponivel;
                float precoadescontar;
                if(indexacao<3) {
                    quantidadedisponivel=SHM->mercado1.acoes[indexacao].quantidade;
                    precoadescontar=SHM->mercado1.acoes[indexacao].valorvenda*quant;
                    if(precoadescontar>SHM->users[indice_cliente].saldo){
                        cleanarray(buffer);
                        sprintf(buffer, "Não tem saldo suficiente\n");
                        write(client, buffer, BUFLEN-1);
                        return;
                    }
                    if(quantidadedisponivel<quant) {
                        SHM->users[indice_cliente].saldo-=precoadescontar;
                        SHM->mercado1.acoes[indexacao].quantidade -= quantidadedisponivel;
                        SHM->users[i].acoes[indexacao]+=quantidadedisponivel;
                    }else{
                        SHM->users[indice_cliente].saldo-=precoadescontar;
                        SHM->mercado2.acoes[indexacao].quantidade -= quant;
                        SHM->users[i].acoes[indexacao]+=quant;
                    }
                }else{
                    quantidadedisponivel=SHM->mercado2.acoes[indexacao-3].quantidade;
                    precoadescontar=SHM->mercado2.acoes[indexacao-3].valorvenda*quant;
                    if(precoadescontar>SHM->users[indice_cliente].saldo){
                        cleanarray(buffer);
                        sprintf(buffer, "Não tem saldo suficiente\n");
                        write(client, buffer, BUFLEN-1);
                        return;
                    }
                    if(quantidadedisponivel<quant) {
                        SHM->users[indice_cliente].saldo-=precoadescontar;
                        SHM->mercado2.acoes[indexacao-3].quantidade -= quantidadedisponivel;
                        SHM->users[i].acoes[indexacao]+=quantidadedisponivel;
                    }
                    else{
                        SHM->users[indice_cliente].saldo-=precoadescontar;
                        SHM->mercado2.acoes[indexacao-3].quantidade -= quant;
                        SHM->users[i].acoes[indexacao]+=quant;
                    }
                }
                cleanarray(buffer);
                sprintf(buffer, "Comprou %d acoes %s com sucesso\n", quant, nomeacao);
                write(client, buffer, BUFLEN-1);
                break;
            }
        }
    }
}

void showinfo(int client, char nome[]){
    char buffer[BUFLEN];
    char bufferaux[BUFLEN];
    int index;
    cleanarray(buffer);
    for (int i = 0; i < MAX_USERS; ++i) {
        if(strcmp(SHM->users[i].nome, nome)==0){
            index=i;
            for (int j = 0; j < 6; ++j) {
                if(j<3 && SHM->users[i].bolsas[0]==1){
                    cleanarray(bufferaux);
                    sprintf(bufferaux, "Mercado: %s, Acao: %s, Quantidade: %d\n", SHM->mercado1.nome,SHM->mercado1.acoes[j].nome, SHM->users[i].acoes[j]);
                    strcat(buffer, bufferaux);
                }else{
                    if(SHM->users[i].bolsas[1]==1) {
                        cleanarray(bufferaux);
                        sprintf(bufferaux, "Mercado: %s, Acao: %s, Quantidade: %d\n", SHM->mercado2.nome,
                        SHM->mercado2.acoes[j - 3].nome, SHM->users[i].acoes[j]);
                        strcat(buffer, bufferaux);
                    }
                }
            }
            break;
        }
    }
    sprintf(bufferaux, "Saldo: %f\n", SHM->users[index].saldo);
    strcat(buffer, bufferaux);
    write(client, buffer, BUFLEN-1);
}


void venda(int client, char user[]){
    int indexacao;
    char buffer[BUFLEN];
    char nomeacao[100];
    int quant;
    int preco;
    int indice_cliente;
    cleanarray(buffer);
    sprintf(buffer, "Insira o nome e numero de ações que quer vender e o preco que deseja vender:\n"
                    "(formato acao-numerodeacoes-preco)\n");
    write(client, buffer, BUFLEN-1);
    int nread = read(client, buffer, BUFLEN-1);
    while(nread<=0){
        nread = read(client, buffer, BUFLEN-1);
    }
    buffer[nread - 1] = '\0';
    strcpy(nomeacao, strtok(buffer, "-"));
    indexacao=findacao(nomeacao);
    if(indexacao>=0){
        quant= atoi(strtok(NULL, "-"));
        if(quant%10!=0&&quant<101){
            cleanarray(buffer);
            sprintf(buffer, "A quantidade que deseja vender deve ser um multiplo de 10 e no maximo 100\n");
            write(client, buffer, BUFLEN-1);
            return;
        }
        preco= atoi(strtok(NULL, "\n"));
    }else{
        cleanarray(buffer);
        sprintf(buffer, "Esta acao nao existe\n");
        write(client, buffer, BUFLEN-1);
        return;
    }
    cleanarray(buffer);
    sprintf(buffer, "Deseja proceder com a operação?(S/N)\n");
    write(client, buffer, BUFLEN-1);
    cleanarray(buffer);
    nread = read(client, buffer, BUFLEN-1);
    while(nread<=0){
        nread = read(client, buffer, BUFLEN-1);
    }
    buffer[nread-1]='\0';

    if(strcmp(buffer, "S")==0){
        for (int i = 0; i < MAX_USERS; ++i) {
            if(strcmp(SHM->users[i].nome, user)==0){
                indice_cliente=i;
                if(quant>SHM->users[i].acoes[indexacao]){
                    cleanarray(buffer);
                    sprintf(buffer, "Você não tem estas ações\n");
                    write(client, buffer, BUFLEN-1);
                    return;
                }else{
                    if(indexacao<3 && preco < SHM->mercado1.acoes[indexacao].valorcompra) {
                        cleanarray(buffer);
                        sprintf(buffer, "Nao existem compradores a pagar o preço que deseja pela ação\n");
                        write(client, buffer, BUFLEN - 1);
                        return;
                    }
                    else{
                        if(preco<SHM->mercado2.acoes[indexacao-3].valorcompra){
                            cleanarray(buffer);
                            sprintf(buffer, "Nao existem compradores a pagar o preço que deseja pela ação\n");
                            write(client, buffer, BUFLEN-1);
                            return;
                        }
                    }
                }
                float precoasomar;
                if(indexacao<3) {
                    precoasomar=SHM->mercado1.acoes[indexacao].valorcompra*quant;
                    SHM->users[indice_cliente].saldo+=precoasomar;
                    SHM->mercado2.acoes[indexacao].quantidade += quant;
                    SHM->users[indice_cliente].acoes[indexacao]-=quant;

                }else{
                    precoasomar=SHM->mercado2.acoes[indexacao-3].valorcompra*quant;
                    SHM->users[indice_cliente].saldo+=precoasomar;
                    SHM->mercado2.acoes[indexacao-3].quantidade += quant;
                    SHM->users[indice_cliente].acoes[indexacao]-=quant;

                }
                cleanarray(buffer);
                sprintf(buffer, "Vendeu %d acoes %s com sucesso\n", quant, nomeacao);
                write(client, buffer, BUFLEN-1);
                return;
            }
        }
    }else{
        cleanarray(buffer);
        sprintf(buffer, "A cancelar...\n");
        write(client, buffer, BUFLEN-1);
    }
}


void processa_cliente(int client, int fd){
 	char buffer[BUFLEN];
 	char user[BUFLEN];
    char pass[BUFLEN];
    cleanarray(pass);
    cleanarray(user);
    cleanarray(buffer);
 	int nread;
 	int n=0;
	while(n<2){//sistema de identificação do user
		if(n==0) {
            sprintf(buffer, "Servidor: Indique o seu nome:\n");
        }
		else{
            sprintf(buffer, "Servidor: Indique a sua password:\n");
		}
        write(client, buffer, BUFLEN-1);
		nread = read(client, buffer, BUFLEN-1);
		if (nread>0){
			buffer[nread-1] = '\0';
            if(n==0) {
                strcpy(user, buffer);
            }
            else{
                strcpy(pass, buffer);
                int check = verifica_user(user, pass);
                if(check==-1){
                    cleanarray(buffer);
                    sprintf(buffer, "Servidor: Login Inválido\n");
                    write(client, buffer, BUFLEN-1);
                    close(client);
                    break;
                }
                else{
                    cleanarray(buffer);
                    sprintf(buffer, "Servidor: Bem Vindo\n");
                    //write(client, buffer, BUFLEN-1);
                    //cleanarray(buffer);
                    char bufferaux[BUFLEN];
                    cleanarray(bufferaux);
                    if(check==0){
                        sprintf(bufferaux, "Não tem acesso a nenhum mercado!\n");
                    }else if(check==1){
                        sprintf(bufferaux, "Tem acesso ao mercado %s!\n", SHM->mercado1.nome);
                    }else if(check==2) {
                        sprintf(bufferaux, "Tem acesso ao mercado %s!\n", SHM->mercado2.nome);
                    } else{
                        sprintf(bufferaux, "Tem acesso aos mercados %s e %s!\n", SHM->mercado1.nome, SHM->mercado2.nome);
                    }
                    strcat(buffer, bufferaux);
                    write(client, buffer, BUFLEN-1);
                }
            }
		}
        cleanarray(buffer);
        n++;
	}
	while(strcmp(buffer, "SAIR")!=0){
        cleanarray(buffer);
        nread = read(client, buffer, BUFLEN-1);
        if (nread>0) {
            buffer[nread - 1] = '\0';
            if(strcmp(buffer, "COMPRAR")==0){
                compra(client, user);
            }
            else if(strcmp(buffer, "VENDER")==0){
                venda(client, user);
            }
            else if(strcmp(buffer, "INFO")==0){
                showinfo(client, user);
            }
            else if(strcmp(buffer,"BOLSAS")==0){
            	enviaaddr(client , user);
            }
            else if(strcmp(buffer, "SAIR")==0){
                cleanarray(buffer);
                close(client);
                kill(getpid(), SIGTERM);
                //sprintf(buffer, "A Sair.....\n");
                //write(client, buffer, BUFLEN-1);
                return;
            }
            else{
                cleanarray(buffer);
                sprintf(buffer, "Comando Inválido\n");
                write(client, buffer, BUFLEN-1);
            }
        }
	}
}




void server_TCP(){
    tcp_pid=getpid();
    signal(SIGINT, SIG_IGN);
	int fd, client;
  	struct sockaddr_in addr, client_addr;
  	int client_addr_size;

    char str[BUFSIZ];
    bzero((void *) &addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORTO_BOLSA);

    if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        erro("na funcao socket");
    if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
        erro("na funcao bind");
    if( listen(fd, 5) < 0)
        erro("na funcao listen");
    client_addr_size = sizeof(client_addr);

	n = 0;
	while (1) {

    	while(waitpid(-1,NULL,WNOHANG)>0){
    	    n--;
    	}
		client = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&client_addr_size);
        printf("%d\n", n);
    	if(n==5){
            //close(fd);
			sprintf(str,"Varios utilizadores ligados...Tente novamente mais tarde\n");
            write(client, str, BUFLEN-1);
            close(client);
		}
		else {
            n++;
		    pid_t pid = fork();
            if (pid == 0) {
                close(fd);
                processa_cliente(client, fd);
            }
        }
  }
    exit(0);
}
