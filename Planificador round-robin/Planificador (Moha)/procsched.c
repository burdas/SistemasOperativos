#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <wait.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "fragmenta.h"
#include <sys/wait.h>
#include <string.h>

int acabado;
int state;
void terminar(int x);
struct mensaje{
	long id;
	int pide;
}mensaje;
void main(int argc,char **argv){
	int pid;
	struct mensaje mi_mensaje;
	int fid;
	char *cadena;
	int clave=ftok("/tmp",22);
	int idcola=msgget(clave,IPC_CREAT|0600);
	mi_mensaje.id=11;
	cadena=(char*)malloc(100*sizeof(char));
	if(argc==3){
		fid=open(argv[2],O_RDONLY,0400);
		dup2(fid,STDIN_FILENO);
		close(fid);
	}
	msgget(11,IPC_CREAT|IPC_PRIVATE|0600);
	while (scanf("%[^\n]",cadena)!=EOF){
			pid=fork();
	if (pid==0){
		char **arg;
		arg=fragmenta(cadena);
		//pause();evitar procesar sigcont poque pause ignora sigcont
		kill(getpid(), SIGSTOP);//me mando a mi mismo parar
		execvp(arg[0],arg);
		//mensaje de error 
		exit(99);
	}else{
		mi_mensaje.pide=pid;
		msgsnd(idcola,&mi_mensaje,sizeof(int),IPC_NOWAIT);
         }
		getchar();
	}
	
	/////
	struct sigaction sa;
	sa.sa_handler=&terminar;
	sa.sa_flags=SA_NOCLDSTOP;
	sigaction(SIGCHLD,&sa,NULL);
	signal(SIGINT, &acabar);
	int cc=0;
	int ciclos=0;
	int p=-1;
	int cuanto=atoi(argv[1]);
	acabado=0;
	while((acabado==0) && (msgrcv(idcola,&mi_mensaje,sizeof(int),mi_mensaje.id,IPC_NOWAIT)!=EOF)){
		state=0;
		ciclos++;
		if(p!=mi_mensaje.pide)
				cc++;
		kill(mi_mensaje.pide,SIGCONT);
		sleep(cuanto);
		if(state==0){
			kill(mi_mensaje.pide,SIGSTOP);
			msgsnd(idcola,&mi_mensaje,sizeof(int),IPC_NOWAIT);
		}
		p=mi_mensaje.pide;	
       	}
	if (acabado==1) 
	while  (msgrcv(idcola,&mi_mensaje,sizeof(int),mi_mensaje.id,IPC_NOWAIT)!=EOF)
		kill(mi_mensaje.pide,SIGTERM);
	
	
printf("En total hubo %d cambios de contexto \n",cc);
printf("En total hubo %d ciclos \n",ciclos);
/////eliminar la cola
msgctl(idcola,IPC_RMID,0);
//
} 


void terminar(int x){
	 state=1;
}

void acabar(int x){
	acabado=1;
}
