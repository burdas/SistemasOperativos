#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void main(int argc, char *argv[]){
        
    //Argumentos
    int periodo = atoi(argv[1]);
    int num_votos = atoi(argv[2]);
    int i = 0;
    //Votar
    srand(getpid());
    while(i < num_votos){
        
        //Esperar
        sleep(periodo);
        
        //Crear voto aleatorio
        int voto = rand()%4+1;
        
        //Mandar voto
        //printf("%d",voto);
        write(STDOUT_FILENO,&voto,sizeof(int));
        
        //Contar voto
        i++;
    }
    
}