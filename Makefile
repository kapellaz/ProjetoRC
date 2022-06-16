FLAGS  = -Wall -lpthread -D_REENTRANT
CC     = gcc
PROG   = server
OBJS   = connectTCP.o connectUDP.o Servidor_RC.o


all:	${PROG}

clean:
	rm ${OBJS} ${PROG} *~

${PROG}:	${OBJS}
	${CC} ${OBJS} ${FLAGS} -o $@

.c.o:
	${CC} ${FLAGS} $< -c

##########################

server: connectTCP.o connectUDP.o Servidor_RC.o

connectTCP.o: connectTCP.c Servidor.h

connectUDP.o: connectUDP.c Servidor.h

Servidor_RC.o: Servidor_RC.c Servidor.h
