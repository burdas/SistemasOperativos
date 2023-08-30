#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

int pid, opcion, segundos = 0, minutos = 0, horas = 0, primera = 1; 

void manejador(int sig){
    if(sig == SIGUSR2){
        segundos = 0;
        minutos = 0;
        horas = 0;
        opcion = 0;
    } else {
      if (opcion == 0)
      {
        opcion = 1;
      } else {
        opcion = 0;
      }
    }
}
int main(){
    pid = fork();
    if (pid == -1){
      printf("Problema en la creación del nuevo proceso");
      exit(EXIT_FAILURE);
    }
    if(pid == 0){
        /*hijo*/
      signal(SIGUSR1, manejador);
      signal(SIGUSR2, manejador);
        while (1)
         {  
            if (primera == 1)
            {
              system("clear");
              printf("\n\n\n\n\n\n\n\n\n\n\t\t\tcontador: %d h %d m %d s\n", horas, minutos, segundos);
              primera = 0;
            }
            if (opcion == 1)
            {
              minutos = minutos + (segundos / 60);
              segundos = segundos % 60;
              horas = horas + (minutos / 60);
              minutos = minutos % 60;

              system("clear");
              printf("\n\n\n\n\n\n\n\n\n\n\t\t\tcontador: %d h %d m %d s\n", horas, minutos, segundos);
              segundos++;
              sleep(1);
            } else {
              system("clear");
              printf("\n\n\n\n\n\n\n\n\n\n\t\t\tcontador: %d h %d m %d s\n", horas, minutos, segundos);
              sleep(1);
            }
         } 
    } else {
        /*padre*/
      char aaa;
      while (1)
         {
            sleep(1);
            aaa = getchar();
            fflush(stdout);
            switch(aaa){
              case '0':
                kill(pid, SIGTERM);
                exit(1);
              case '1':
                kill(pid, SIGUSR1);
                break;
              case '2':
                kill(pid, SIGUSR2);
                break;

            }
         } 
    }
    
}