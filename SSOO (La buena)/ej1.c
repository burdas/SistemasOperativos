#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct mensaje { 
               int secuencia,pidEmisor;     
          } mensaje;

int main(){
    int pid, tuberia1[2], tuberia2[2], pip1, pip2;
    struct mensaje mens;
    //tuberia[2][2]
    pip1 = pipe(tuberia1);    //comprobar
    if (pip1 == -1)
    {
      printf("Problema en la creación de la primera tubería\n");
      exit(-1);
    }
    pip2 = pipe(tuberia2);
    if (pip2 == -1)
    {
      printf("Problema en la creación de la segunda tubería\n");
      exit(-1);
    }
    pid = fork();
    if(pid < -1){
        printf("El programa ha dejado de funcionar");
        exit(-1);
    }
    if(pid == 0){
        /*hijo*/
          close(tuberia1[1]);
          close(tuberia2[0]); 
          for(int i=0;i<5;i++){
              mens.secuencia = getpid();
              read(tuberia1[0], &mens, sizeof(mens));
              mens.secuencia++;
              printf("Número de secuencia: %d\n",mens.secuencia);
              write(tuberia2[1], &mens, sizeof(mens));
        }
          close(tuberia1[0]);
          close(tuberia2[1]);
          printf("Aupa!!!!\n");
          
          exit(1);  
    } else {
        /*padre*/
        mens.secuencia = 0;
        close(tuberia1[0]);
        close(tuberia2[1]);
        for(int i=0;i<5;i++){
              mens.secuencia = getpid();
              write(tuberia1[1], &mens, sizeof(mens));
              mens.secuencia++;
              printf("Número de secuencia: %d\n",mens.secuencia);
              read(tuberia2[0], &mens, sizeof(mens));
        }
        close(tuberia1[1]);
        close(tuberia2[0]);
        printf("Alegria!!! %d \n", mens.secuencia);
 
    }
    
}