/****************************************
Autor: Marcos Burdaspar Celada
Fecha: 19/12/2016
*****************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sem.h>

int contador = 1;

void contar(int x){
    contador = 0;
}

void main(int argc, char *argv[]){   
    //Zona de argumentos
    int periodo = atoi(argv[1]);
    int claveIntroducida = atoi(argv[2]);
    int ID = atoi(argv[3]);
    
    //Zona de memoria compartida
    signal(SIGINT, &contar);
    key_t clave = ftok("./",claveIntroducida); //Se crea la clave con el argumento introducido
    int idMemoria;
    idMemoria=shmget(clave, 0, 0);
    const int *votos;
    votos = (int *)shmat(idMemoria, NULL, SHM_RDONLY);
    int idSemaforo; // Se crea el semaforo
    idSemaforo=semget(clave, 4, 0);

    struct sembuf accion; // Se inicializa el semaforo
    accion.sem_num = ID; 
    accion.sem_flg = 0;  
    
    while(contador == 1){
        sleep(periodo);
        accion.sem_op = -1;  
        semop(idSemaforo, &accion, 0);
        printf("El candidato %d obtiene %d votos\n",ID, votos[ID-1]);
        semop(idSemaforo, &accion, 0); 
    }
}
