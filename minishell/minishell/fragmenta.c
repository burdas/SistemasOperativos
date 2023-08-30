#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
char **fragmenta(const char *cadena){
    char**arg,*token,*copia;
    int cont=1,i=0;
    copia=(char*)malloc((strlen(cadena)+1)*sizeof(char));
    strcpy(copia,cadena);
    token=strtok((char*)cadena," ");
    
    while(token!=NULL){
        if (strlen(token)>0) 
            cont ++;
        token=strtok(NULL," ");
    }
    arg=(char**)malloc(cont*sizeof(char*));
    
    token=strtok(copia," ");
    while(token!=NULL){
        if (strlen(token)>0){
            arg[i]=(char*)malloc((strlen(token)+1)*sizeof(char));
            strcpy(arg[i],token); 
        }
        i++;
        token=strtok(NULL," ");
    }
    arg[i]=NULL;
    return(arg);
}
void borrag(char **arg){
	int i=0;
    while(arg[i]!=NULL){
        free(arg[i]);
        i++;    
    }
    free(arg);
}
