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


#define BUFLEN 1024
#define key 1234
#define IP_SERVER "127.0.0.1"
#define BOLSA1 "239.0.0.1"
#define BOLSA2 "239.0.0.2"
#define MAX_USERS 10
#define NOME_USERS 100
#define N_BOLSAS 2
#define MAS_USERS 10
#define PORTO_MULTICAST1 7777
#define PORTO_MULTICAST2 8888
struct admin{
	char nome[NOME_USERS];
	char pass[NOME_USERS];
};

struct acao{
    int index;
	char nome[NOME_USERS];
	float valorvenda;
	float valorcompra;
	int quantidade;
};

struct user{
	bool existe;
	char nome[NOME_USERS];
	char pass[NOME_USERS];
	int bolsas[2];//permite saber a que bolsas tem acesso
	int acoes[6];//açoes que contém em cada bolsa
	float saldo;
};

struct bolsa{
	char nome [NOME_USERS];//nome da bolsa
	struct acao acoes[3];
};

struct bloco{
	int REFRESHTIME;
	struct user users[MAX_USERS];
	struct bolsa mercado1;
	struct bolsa mercado2;
    int n;
};


pid_t tcp_pid,child_pid, pidmain;

int n,PORTO_CONFIG, PORTO_BOLSA;
int id_shm, acoes_b1, acoes_b2;
struct admin admin;
struct bloco *SHM;
void erro(char *s);
void server_TCP();
void server_UDP();
