/********************************************
Autor: Marcos Burdaspar Celada
Fecha: 15/12/2017
*********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define CANDIDATOS 4                                // Número de candidatos

void main(int argc, char *argv[]){
    int periodo = atoi(argv[1]);                    // Guardamos el periodo
    int numVotos = atoi(argv[2]);                   // Guardamos el número votos que se pueden generar
    
    int i = 0;
    srand(getpid());                                // Hacemos que la semilla cambie (usamos el pid actual)
    while(i < numVotos){
        fflush(stdout);                             // Limpiamos el buffer de entrada
        sleep(periodo);                             // Esperamos el periodo introducido
        int voto = (rand()%CANDIDATOS)+1;           // Genera un voto aleatorio entre el 1 y el 4
        write(STDOUT_FILENO,&voto,sizeof(int));     // Sacamos por la salida estandar el voto obtenido
        i++;
    }
}
