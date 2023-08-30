/*Librerias para visor*/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sem.h>
/*Variables globales*/
int terminar=0;
void finalBucle(int x);
/*Programa visor*/
int main(int argc, char **argv)
{
	/*Variables*/
	int memoriaID;
	int *votos;
	int semaforoID;

	/*Recogemos el periodo, la clave y el id*/
	int periodo=atoi(argv[1]);
	int clave=atoi(argv[2]);
	int id=atoi(argv[3]);

	/*Vinculamos la memoria: puntero para poder acceder a ella*/
	memoriaID=shmget(clave,0,0);
	votos = (int*)shmat(memoriaID,NULL,SHM_RDONLY);

	/*Obtener el id del SEMAFORO*/
	semaforoID= semget(clave,4,0);
	/*Estructura del semaforo*/
	struct sembuf accion;
	accion.sem_num=id;// indice del semaforo
	accion.sem_flg=0;
	/*Señal de interrupcion del bucle while*/
	signal(SIGINT,&finalBucle);
	//sleep(2);
	while(terminar==0){
		sleep(periodo);
		/*Hacemos un WAIT sobre el semaforo*/
		accion.sem_op=-1;
		semop(semaforoID,&accion,id);
		printf("El candidato %d obtiene %d votos\n",id,votos[id-1]);

		accion.sem_op = 1;
		semop(semaforoID,&accion,1);
	}
	sleep(2);
}
void finalBucle(int x){
	terminar=1;
}
