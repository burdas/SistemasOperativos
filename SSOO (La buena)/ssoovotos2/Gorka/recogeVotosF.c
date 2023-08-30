#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO*/
};

/*Variables globales*/
int terminar = 0;
void final(int x){
	sleep(2);
	terminar = 1;
}

/*PROGRAMA PRINCIPAL*/
void main(int argc, char *argv[]){
	printf("Nombre del Programa: RecogeVotos\n");
	printf("Autor: Gaspar Blazquez, Gorka\n");
	printf("Asignatura: Sistemas Operativos\n");
	printf("Fecha: 14-diciembre-2017\n");
	printf("*************************************\n");
	printf("\n");
	/*Recogemos los argumentos*/
	int clave = atoi(argv[1]);
	int numVotante = atoi(argv[2]);
	int periodo = atoi(argv[3]);
	int numVotos = atoi(argv[4]);

	/*Creamos la memoria compartida*/
	printf("\nSE ESTA CREANDO LA MEMORIA\n");
	int memoriaID = shmget(clave,(numVotante+1)*sizeof(int),0666|IPC_CREAT|IPC_EXCL);
	if(memoriaID < 0){
		printf("Error a la hora de crear la memoria");
		exit(-1);
	}
	else{
		printf("La memoria compartida se ha creado correctamente con un id: %d\n",memoriaID);
	}

	/*Creamos el semaforo*/
	printf("\nSE ESTA CREANDO EL SEMAFORO\n");
	int semaforoID = semget(clave,4,0600|IPC_CREAT);
	if(semaforoID < 0){
		printf("Error a la hora de crear el semaforo");
		exit(-1);
	}
	else{
		printf("El semaforo se ha creado correctamente con un id: %d\n",semaforoID);
	}
	/*Necesitamos tantos semaforos como votantes, es decir, 4*/
	/*Inicializamos los semaforos*/
	union semun num;
	num.val = 1;
	semctl(semaforoID,0,SETVAL,num);
	semctl(semaforoID,1,SETVAL,num);
	semctl(semaforoID,2,SETVAL,num);
	semctl(semaforoID,3,SETVAL,num);

	/*Votantes*/
	printf("\nESTOS SON LOS VOTANTES: \n");
	/*Necesitamos tuberias para la comunicacion*/
	/*Tantas tuberias como numVotantes*/
	int tuberia[numVotante][2];
	int arrayPidVotante[numVotante];
	pid_t pid;
	for(int i=0;i<numVotante;i++){
		pipe(tuberia[i]);
		fcntl(tuberia[i][0],F_SETFL,O_NONBLOCK);
	}

	
	for(int i=0;i<numVotante;i++){
		/*Lectura no bloqueante*/
		pid = 0;
		pid = fork();
		if (pid == 0){
			dup2(tuberia[i][1],STDOUT_FILENO);
			close(tuberia[i][0]);
			close(tuberia[i][1]);
			/*Cierra el resto de tuberias que no uso*/
			for(int j=0;j<numVotante;j++){
				if(i != j){
					close(tuberia[j][0]);
					close(tuberia[j][1]);
				}
			}
			execl("./votante","./votante",argv[3],argv[4],NULL);
		}
		else{
			arrayPidVotante[i] = pid;
			printf("Votante: %d\n",pid);
		}
	}
	sleep(2);

	/*Visor*/
	printf("\nESTOS SON LOS VISORES: \n");
	int arrayPidVisor[4];
	for(int i=0;i<4;i++){
		pid = 0;
		pid = fork();
		if(pid == 0){
			/*El valor del pid guardado en i es un entero por lo que hay que transformarlo a char para poder pasarlo como argumento*/
			char cast = '0' + i +1;
			execl("./visor","./visor",argv[3],argv[1],&cast,NULL);
		}
		else{
			arrayPidVisor[i] = pid;
			printf("Visor %d\n", pid);
		}	
	}
	/*Conjunto vacio para guardar descriptores*/
	fd_set s;
	FD_ZERO(&s);

	/*Es necesario cerrar todas las tuberias en escritura*/
	/*Guardamos en el conjunto s aquellas tuberias en lectura que nos interesan*/
	for(int i=0;i<numVotante;i++){
		close(tuberia[i][1]);
		FD_SET(tuberia[i][0],&s);
	}	

	/*Señal para interrumpir la ejecucion del programa cuando pulsamos CTRL+C*/
	signal(SIGINT,&final);

	/*Vinculamos la memoria*/
	int *nVotos;
	nVotos = (int*)shmat(memoriaID,NULL,0);

	/*Se inicializa el semaforo*/
	struct sembuf accion;
	accion.sem_flg = 0;

	printf("\n......................\n");
	/*Realizamos el recuento*/
	sleep(2);
	printf("\nRECUENTO DE LA VOTACION\n");
	int sumaVoto = 0;
	int voto;
	int totalVotos = numVotante*numVotos;
	while((terminar==0) && (sumaVoto < (numVotante*numVotos))){
		/*Seleccionamos del conjunto s las tuberias*/
		select(tuberia[numVotante-1][0]+1,&s,NULL,NULL,NULL);
		for(int i=0;i<numVotante;i++){
			if(FD_ISSET(tuberia[i][0],&s)!=-1){
				FD_CLR(tuberia[i][0],&s);
				FD_SET(tuberia[i][0],&s);
				read(tuberia[i][0],&voto,sizeof(int));
				accion.sem_num = voto-1;
				accion.sem_op = -1;
				semop(semaforoID,&accion,1);

				/*Recuento de votos*/
				nVotos[voto-1]++;
				sumaVoto++;

				accion.sem_op = 1;
				semop(semaforoID,&accion,1);
			}
		}
	}
	
	printf("\n.....................\n");
	printf("\nFIN DE LA VOTACION\n");
	sleep(2);

	/*Eliminando votantes y visores*/
	printf("\nELIMANDO VOTANTES.....\n");
	for (int i = 0; i < numVotante; i++){
		kill(arrayPidVotante[i],SIGINT);
		wait(NULL);
	}

	printf("\nELIMANDO VISORES.....\n");
	for (int i = 0; i < 4; i++){
		kill(arrayPidVisor[i],SIGTERM);
		wait(NULL);
	}

	/*Resultados de la votacion*/
	printf("\nRESULTADOS DE LA VOTACION: \n");
	for (int i = 0; i < 4; i++){
		printf("El candidado %d obtiene %d votos\n",i+1,nVotos[i]);
	}
	printf("Participacion: %.2f%c\n",((float)sumaVoto)*100/totalVotos,'%');

	/*Borramos los recursos*/
	printf("\nELIMINANDO RECURSOS USADOS......\n");
	/*Eliminar semaforo*/
	semaforoID = semget(clave,2,0);
	if(semctl(semaforoID,0,IPC_RMID)<0){
		printf("Error a la hora de borrar el semaforo\n");
	}
	printf("\nFIN DEL PROGRAMA\n");

	/*Eliminar memoria*/
	memoriaID = shmget(clave,2,0);
	if(shmctl(memoriaID,IPC_RMID,NULL)<0){
		printf("Error a la hora de eliminar la memoria\n");
	}

	/*Cerramos tuberias*/
	for (int i = 0; i < numVotante; i++){
		close(tuberia[i][0]);
	}
}	
