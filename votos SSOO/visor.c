#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/sem.h>

int contar = 1;

void escrutinio(int x){
    contar = 0;
}

void main(int argc, char *argv[]){
    
    //Argumentos
    int periodo = atoi(argv[1]);
    int clave_n = atoi(argv[2]);
    int ID = atoi(argv[3]);
    
    //MEMORIA COMPARTIDA
    //Configurar SIGINT
    signal(SIGINT, &escrutinio);
    //Creacion de la clave
    key_t clave = ftok("./",clave_n);
    //Vincular memoria
    int id_memoria;
    id_memoria=shmget(clave, 0, 0);
    const int *votos;
    votos = (int *)shmat(id_memoria, NULL, SHM_RDONLY);
    //Vincular semaforo
    int id_semaforo;
    id_semaforo=semget(clave, 4, 0);
    //Inicializar mensaje semaforo
    struct sembuf accion;
    accion.sem_num = ID; 
    accion.sem_flg = 0;  
    
    //Bucle
    while(contar == 1){
        //Espera
        sleep(periodo);
        //Read lock
        // WAIT sobre el semáforo 
        accion.sem_op = -1; // operación wait (enteros negativos)     
        semop(id_semaforo, &accion, 0);
        
        //Lectura
        printf("El candidato %d obtiene %d votos\n",ID, votos[ID-1]);
        
        //read unlock
        // SIGNAL sobre el semáforo  
        accion.sem_op = 1;  // operación signal (enteros positivos) 
        semop(id_semaforo, &accion, 0); 
    }
}