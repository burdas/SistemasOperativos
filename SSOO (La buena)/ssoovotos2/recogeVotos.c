/********************************************
Autor: Marcos Burdaspar Celada
Fecha 15/12/2017
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

#define CANDIDATOS 4

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
    int clave = ftok("./",clave2);   //Creacion de la clave
    int idMemoria;     //Creacion de la memoria
    idMemoria=shmget(clave, (CANDIDATOS+1)*sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);
    if(idMemoria < 0){
        printf("Error en la creación de la memoria compartida\n");
        exit(EXIT_FAILURE);
    }else{
        printf("Memoria compartida ID[%d]\n",idMemoria);
    }
    int idSemaforo;     //Creacion del semaforo
    idSemaforo = semget(clave, CANDIDATOS, 0600 | IPC_CREAT);
    if(idSemaforo < 0){
        printf("Error en la creación del semaforo\n");
        exit(EXIT_FAILURE);
    }else{
        printf("semaforo ID:[%d]\n",idSemaforo);
    }
    //Inicializando el semaforo
    union semun valor;
    valor.val = 1;
    for (int i = 0; i < CANDIDATOS; ++i)
    {
        semctl(idSemaforo,i,SETVAL,valor);
    }

    printf("\nCreacion de los votantes...\n\n");
    int tuberia[numVotantes][2];
    for (int i = 0; i < numVotantes; ++i)
    {
        pipe(tuberia[i]);
        fcntl(tuberia[i][0], F_SETFL, O_NONBLOCK);
    }
    int i;
    for(i=0;i<numVotantes;i++){
        int pid = 0;

        pid = fork();
        if(pid == 0){
            //Proceso hijo
            close(tuberia[i][0]);
            dup2(tuberia[i][1], STDOUT_FILENO);
            close(tuberia[i][1]);
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
    int PID_visores[CANDIDATOS];
    for(i=0;i<CANDIDATOS;i++){
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
    
    signal(SIGINT, &fin_contador);
    int *votos;
    votos = (int *)shmat(idMemoria,NULL, 0);
    struct sembuf accion;
    accion.sem_flg = 0;  

    sleep(1);
    printf("\nComenzando la votacion...\n\n");
    fd_set descriptoresLectura;
    FD_ZERO(&descriptoresLectura);
    for (int i = 0; i < numVotantes; ++i)
    {
        close(tuberia[i][1]);
        FD_SET(tuberia[i][0], &descriptoresLectura);
    }
    int voto;
    while(contador == 1 && sumaVotos<tVotos){
        select(tuberia[numVotantes - 1][0] + 1, &descriptoresLectura, NULL, NULL, NULL);
        for (int i = 0; i < numVotantes; ++i)
        {
            if (FD_ISSET(tuberia[i][0],&descriptoresLectura) != -1)
            {
                FD_CLR(tuberia[i][0],&descriptoresLectura);
                read(tuberia[i][0],&voto,sizeof(int));
                FD_SET(tuberia[i][0],&descriptoresLectura);
                accion.sem_num = voto;
                accion.sem_op = -1;
                semop(idSemaforo, &accion, 0);
                votos[voto-1]++; //contar el voto
                sumaVotos++;
                accion.sem_op = 1;
                semop(idSemaforo, &accion, 0);
            }
        }
    }
    printf("\nFin de la votacion\n\n");
    
                //SALIR//
    printf("\nEliminando los votantes...\n\n");
    for(i=0;i<numVotantes;i++){
        kill(PID_votante[i],SIGINT);
        wait(NULL);
    }
    printf("\nEliminando los visores...\n\n");
    for(i=0;i<CANDIDATOS;i++){
        kill(PID_visores[i],SIGINT);
        wait(NULL);
    }
    system("clear");
    printf("Votantes y visores eliminados con éxito...\n");
    printf("\n-- Resultados finales --\n\n");
    for(i=0;i<CANDIDATOS;i++){
        printf("El candidato %d obtiene %d votos\n",i+1, votos[i]);
    }
    printf("\nTotal de participacion: %.2f%c\n",((float)sumaVotos)*100/tVotos,'%');
    printf("\nEliminando la memoria y los semaforos...\n\n");
    idSemaforo=semget(clave, 2, 0);
    if(semctl( idSemaforo, 0, IPC_RMID) < 0){
        printf("Error en sem_delete\n");
    }else{
        printf("Semaforo eliminado con éxito\n");
    }
    idMemoria=shmget(clave, 2, 0);
    if(shmctl(idMemoria, IPC_RMID, NULL) < 0){
        printf("\nError en shm_delete\n");
    }else{
        printf("Memoria compartida eliminada con éxito\n");
    }
    
}
