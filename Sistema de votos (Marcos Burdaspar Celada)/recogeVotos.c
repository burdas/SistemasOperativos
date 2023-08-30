/********************************************
Autor: Marcos Burdaspar Celada
Fecha 22/12/2016
*********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

union semun {
    int val;    
    struct semid_ds *buf;   
    unsigned short  *array;  
    struct seminfo  *__buf;  
};
int contador = 1;

void fin_contador(int x){
    sleep(1);
    contador = 0;
}

void main(int argc, char *argv[]){
    //Zona de argumentos
    char *claveIntroducida = argv[1];
    int numVotantes = atoi(argv[2]);
    int PID_votante[numVotantes];
    char *periodo = argv[3];
    char * numVotos_s = argv[4];
    int numVotos = atoi(numVotos_s);
    int tVotos = numVotantes*numVotos;
    int sumaVotos = 0;
    
    //Zona de la memoria compartida
    printf("\nCreacion de la memoria y los semaforos...\n\n");
    //Variables
    int clave2 = atoi(claveIntroducida);
    int tamano = 4;
    int clave = ftok("./",clave2);   //Creacion de la clave
    int idMemoria;     //Creacion de la memoria
    idMemoria=shmget(clave, (tamano+1)*sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);
    if(idMemoria < 0){
        printf("Error en shm_create\n");
    }else{
        printf("shm_created[%d]\n",idMemoria);
    }
    int idSemaforo;     //Creacion del semaforo
    idSemaforo = semget(clave, 4, 0600 | IPC_CREAT);
    if(idSemaforo < 0){
        printf("Error en sem_create\n");
    }else{
        printf("sem_created[%d]\n",idSemaforo);
    }
    //Inicializando el semaforo
    union semun valor;
    valor.val = 1;
    semctl(idSemaforo,0,SETVAL,valor);
    semctl(idSemaforo,1,SETVAL,valor);
    semctl(idSemaforo,2,SETVAL,valor);
    semctl(idSemaforo,3,SETVAL,valor);

    printf("\nCreacion de los votantes...\n\n");
    int tuberia[2];
    pipe(tuberia);
    fcntl(tuberia[0], F_SETFL, O_NONBLOCK);
    int i;
    for(i=0;i<numVotantes;i++){
        int pid = 0;
        pid = fork();
        if(pid == 0){
            //Proceso hijo
            dup2(tuberia[1], STDOUT_FILENO);
            execl("./votante","./votante",periodo,numVotos_s,NULL);
        }else{
            //Proceso padre
            PID_votante[i] = pid;
            printf("Votante [%d]\n",pid);
            pid = 0;
        }
    }
    sleep(1);    
    printf("\nCreación de los visores...\n\n");
    int PID_visores[4];
    for(i=0;i<4;i++){
        int pid = 0;
        pid = fork();
        if(pid == 0){
            //Proceso Hijo
            char x = '0'+ i + 1;
            execl("./visor","./visor",periodo,claveIntroducida,&x,NULL);
        }else{
            //Proceso Padre
            PID_visores[i] = pid;
            printf("Visor [%d]\n",pid);
            pid = 0;
        }
    }
    
    close(tuberia[1]); //Cerrar la tuberia no usada
    signal(SIGINT, &fin_contador);
    int *votos;
    votos = (int *)shmat(idMemoria,NULL, 0);
    struct sembuf accion;
    accion.sem_flg = 0;  

    sleep(1);
    printf("\nComenzando la votacion...\n\n");
    int voto;
    while((contador == 1) & (sumaVotos < tVotos)){
        int voto;
        if(read(tuberia[0],&voto,sizeof(int)) != -1){
            accion.sem_num = voto;
            accion.sem_op = -1;
            semop(idSemaforo, &accion, 0);
            votos[voto-1]++; //contar el voto
            sumaVotos++;
            accion.sem_op = 1;
            semop(idSemaforo, &accion, 0);
        }
    }
    printf("\nFin de la votacion\n\n");
    
                //SALIR//
    printf("\nEliminando los votatnes...\n\n");
    for(i=0;i<numVotantes;i++){
        kill(PID_votante[i],SIGINT);
        wait(NULL);
    }
    printf("\nEliminando los visores...\n\n");
    for(i=0;i<4;i++){
        kill(PID_visores[i],SIGINT);
        wait(NULL);
    }
    printf("\n-- Resultados finales --\n\n");
    for(i=0;i<4;i++){
        printf("El candidato %d obtiene %d votos\n",i+1, votos[i]);
    }
    printf("\nTotal de participacion: %.2f%c\n",((float)sumaVotos)*100/tVotos,'%');
    printf("\nEliminando la memoria y los semaforos...\n\n");
    idSemaforo=semget(clave, 2, 0);
    if(semctl( idSemaforo, 0, IPC_RMID) < 0){
        printf("Error en sem_delete\n");
    }else{
        printf("sem_deleted\n");
    }
    idMemoria=shmget(clave, 2, 0);
    if(shmctl(idMemoria, IPC_RMID, NULL) < 0){
        printf("\nError en shm_delete\n");
    }else{
        printf("shm_deleted\n");
    }
    close(tuberia[0]);
    
}
