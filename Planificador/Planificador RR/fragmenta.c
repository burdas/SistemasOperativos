#include <stdlib.h>
#include <string.h>

char **fragmenta(const char *cadena){
    char **arg,*token, *copia;
    int cont = 1;
    int i = 0;
    copia = (char*)malloc((strlen(cadena)+1)*sizeof(char));
    strcpy(copia, cadena);
    token = strtok(copia, " ");
    while (token!=NULL){
        if(strlen(token)>0)
            cont++;
        token = strtok(NULL," ");
    }
    arg = (char**)malloc(cont*(sizeof(char*)));
    strcpy(copia, cadena);
    token = strtok(copia, " ");
    while (token!=NULL){
        if(strlen(token)>0){
            arg[i] = (char*)malloc(strlen(token)+1);
            strcpy(arg[i],token);
            i++;
        }
        token = strtok(NULL," ");
    }
    arg[cont] = NULL;
    
    return arg;
}

void borrarg(char **arg){
    int i = 0;
    while(arg[i]!=NULL){
        free(arg[i]);
        i++;
    }
    free(arg);
}
