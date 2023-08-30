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
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO*/
};


int contar = 1;

void fin_contar(int x){
    sleep(1);
    contar = 0;
}

void main(int argc, char *argv[]){
    
    //Argumentos
    char *clave_s = argv[1];
    int numVotantes = atoi(argv[2]);
    int PID_votante[numVotantes];
    char *periodo = argv[3];
    char * numVotos_s = argv[4];
    int numVotos = atoi(numVotos_s);
    int t_votos = numVotantes*numVotos;
    int sum_votos = 0;
    
    //MEMORIA COMPARTIDA
    printf("\nCREACION MEMORIA Y SEMAFOROS:\n");
    //Variables
    int clave_n = atoi(clave_s);
    int tamano = 4;
    //Creacion de la clave
    int clave = ftok("./",clave_n);
    //Creacion de la memoria
    int id_memoria;
    id_memoria=shmget(clave, (tamano+1)*sizeof(int), 0666 | IPC_CREAT | IPC_EXCL);
    if(id_memoria < 0){
        printf("Error_shm_create\n");
    }else{
        printf("shm_created[%d]\n",id_memoria);
    }
    //Creacion del semaforo
    int id_semaforo;
    id_semaforo = semget(clave, 4, 0600 | IPC_CREAT);
    if(id_semaforo < 0){
        printf("Error_sem_create\n");
    }else{
        printf("sem_created[%d]\n",id_semaforo);
    }
    //Inicializar el semaforo
    union semun valor;
    valor.val = 1;
    semctl(id_semaforo,0,SETVAL,valor);
    semctl(id_semaforo,1,SETVAL,valor);
    semctl(id_semaforo,2,SETVAL,valor);
    semctl(id_semaforo,3,SETVAL,valor);
    
    //VOTANTES
    printf("\nCREACION VOTANTES:\n");
    //Crear tuberias
    int tuberia[2];
    pipe(tuberia);
    fcntl(tuberia[0], F_SETFL, O_NONBLOCK); // Lectura no bloqueante
    //Crear votantes
    int i;
    for(i=0;i<numVotantes;i++){
        int pid = 0;
        pid = fork();
        if(pid == 0){//Hijo
            //Redirigir STDOUT
            dup2(tuberia[1], STDOUT_FILENO);
            //Ejecutar votante
            execl("./votante","./votante",periodo,numVotos_s,NULL);
        }else{//Padre
            //Almacenar PID
            PID_votante[i] = pid;
            printf("Votante [%d]\n",pid);
            pid = 0;
        }
    }
    
    //VISORES
    sleep(1);         //Dar tiempo al conteo de todos los votos realizados
    printf("\nCREACION VISORES:\n");
    int PID_visores[4];
    for(i=0;i<4;i++){
        int pid = 0;
        pid = fork();
        if(pid == 0){//Hijo
            //Ejecutar visor
            /*char ID[4];
            itoa(i,ID,4);*/
            char x = '0'+ i + 1;
            execl("./visor","./visor",periodo,clave_s,&x,NULL);
        }else{//Padre
            //Almacenar PID
            PID_visores[i] = pid;
            printf("Visor [%d]\n",pid);
            pid = 0;
        }
    }
    
    //RECUENTO
    //Cerrar tuberia no usada
    close(tuberia[1]);
    //Configurar SIGINT
    signal(SIGINT, &fin_contar);
    //Vincular memoria
    int *votos;
    votos = (int *)shmat(id_memoria,NULL, 0);
    //Inicializar mensaje semaforo
    struct sembuf accion;
    accion.sem_flg = 0;  
    //Conteo
    sleep(1);
    printf("\nCOMIENZO VOTACION\n");
    int voto;
    while((contar == 1) & (sum_votos < t_votos)){
        int voto;
        if(read(tuberia[0],&voto,sizeof(int)) != -1){
            //Write lock
            // WAIT sobre el semáforo
            accion.sem_num = voto;
            accion.sem_op = -1; // operación wait (enteros negativos)
            semop(id_semaforo, &accion, 0);
            
            //Contar voto
            votos[voto-1]++;
            sum_votos++;
            
            //Write unlock
            // SIGNAL sobre el semáforo  
            accion.sem_op = 1;  // operación signal (enteros positivos) 
            semop(id_semaforo, &accion, 0);
        }
    }
    printf("\nFIN VOTACION\n");
    
    //SALIR
    //Finalizar votantes
    printf("\nELIMINACION VOTANTES...\n");
    for(i=0;i<numVotantes;i++){
        kill(PID_votante[i],SIGINT);
        wait(NULL);
    }
    //Finalizar visores
    printf("\nELIMINACION VISORES...\n");
    for(i=0;i<4;i++){
        kill(PID_visores[i],SIGINT);
        wait(NULL);
    }
    //Escrutinio
    printf("\nRESULTADOS:\n");
    for(i=0;i<4;i++){
        printf("El candidato %d obtiene %d votos\n",i+1, votos[i]);
    }
    printf("Participacion: %.2f%c\n",((float)sum_votos)*100/t_votos,'%');
    
    //Borrar semaforos
    printf("\nELIMINACION MEMORIA Y SEMAFOROS:\n");
    id_semaforo=semget(clave, 2, 0);
    if(semctl( id_semaforo, 0, IPC_RMID) < 0){
        printf("Error_sem_delete\n");
    }else{
        printf("sem_deleted\n");
    }
    //Borrar memoria
    id_memoria=shmget(clave, 2, 0);
    if(shmctl(id_memoria, IPC_RMID, NULL) < 0){
        printf("\nError_shm_delete\n");
    }else{
        printf("shm_deleted\n");
    }
    //Cerrar tuberias
    close(tuberia[0]);
    
}