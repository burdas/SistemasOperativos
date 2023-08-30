/********************************************
Autor: Marcos Burdaspar Celada
Fecha: 12/12/2016
*********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void main(int argc, char *argv[]){
    // Zona de Argumentos
    int periodo = atoi(argv[1]);
    int numVotos = atoi(argv[2]);
    
    int i = 0;
    srand(getpid());
    while(i < numVotos){
        sleep(periodo);
        int voto = rand()%4+1; // Genera un voto aleatorio
        write(STDOUT_FILENO,&voto,sizeof(int));
        i++;
    }
}
