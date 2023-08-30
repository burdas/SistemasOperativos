#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/msg.h>
#include <wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "fragmenta.h"
struct msgStruct{
	long id;
	int pid;
}msgStruct;

int estado, finalizado;
void acabar(int y);
void finalizar(int y);
//El primer argumento es el quanto y el segundo es el nombre del fichero
void main(int argc, char** argv){
	
	if (argc < 3){
		printf("Argumentos Erroneos");
		exit(EXIT_FAILURE);
	}	
	int idFichero, cambioscontexto, ciclos, pidAnterior, pid, quanto, key, msqid;
	struct msgStruct mensaje;
	quanto = atoi(argv[1]);
	key = ftok("/etc", 5);
	msqid = msgget(key, IPC_CREAT|0600);
	char *buffer = (char*)malloc(200 * sizeof(char));
	
	cambiosDeContexto = 0;
	ciclos = 0;
	pidAnterior = -1;
	finalizado = 0;
	mensaje.id = 4;
	if(argc == 3){
		idFichero = open(argv[2], O_RDONLY);
		dup2(idFichero, STDIN_FILENO);
		close(idFichero);
	}
	msgget(4, IPC_CREAT|IPC_PRIVATE|0600);
	while(scanf("%[^\n]", buffer) != EOF){
		pid = fork();
		if (pid == -1){
			printf("Error en la creacion del hijo\n");
			exit(EXIT_FAILURE);
		}
		
		if (pid == 0){
			char **args;
			args = fragmenta(buffer);
			kill(getpid(), SIGSTOP);
			execvp(args[0], args);
			exit(getpid()); 
		}else{
			mensaje.pid = pid;
			msgsnd(msqid, &mensaje, sizeof(int), IPC_NOWAIT);
		}
		getchar();
	}
	struct sigaction accion;
	
	accion.sa_flags = SA_NOCLDSTOP;
	accion.sa_handler = &acabar;

	sigaction(SIGCHLD, &accion, NULL);
	signal(SIGINT, &finalizar);
	
	while ((msgrcv(msqid, &mensaje, sizeof(int), mensaje.id, IPC_NOWAIT) != EOF) && (!finalizado)){
		ciclos++;
		estado = 0;
		if(pidAnterior != mensaje.pid){
			cambioscontexto++;
		}
		kill(mensaje.pid, SIGCONT);
		sleep(quanto);
		if (!estado){
			kill(mensaje.pid, SIGSTOP);
			msgsnd(msqid, &mensaje, sizeof(int), IPC_NOWAIT);
		}
		pidAnterior = mensaje.pid;
	}

	if (finalizado){
		while(msgrcv(msqid, &mensaje, sizeof(int), mensaje.id, IPC_NOWAIT) != EOF){
			kill(mensaje.pid, SIGTERM);
		}
	}
	printf("Se han obtenido %d ciclos y %d cambios de contexto.\n", ciclos, cambioscontexto);
	msgctl(msqid, IPC_RMID, 0);
}

void acabar(int y){
	estado = 1;
}
void finalizar(int y){
	finalizado = 1;
}
