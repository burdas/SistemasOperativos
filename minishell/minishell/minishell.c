#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <fcntl.h>
#include <sys/wait.h>
#include "fragmenta.h"


void main(){
    int pid, i, fid;
    char *cadena, **arg1, **arg2;
    int tuberia[2];
    cadena=(char*)malloc(100*sizeof(char)); 
    while(1){
	    printf("minishell\\>"); //El prompt personalizado tiene doble \ pues con una saltaba una advertencia
	    scanf("%[^\n]",cadena);
	    getchar();
	    if (strcmp(cadena,"exit")==0) 
		exit(777);
	   
	    pid=fork();
	    if (pid==0){
		arg1=fragmenta(cadena);
		i = 0;
		while(arg1[i] != NULL){
			if(strcmp(arg1[i], "|") == 0){
				pipe(tuberia);
				pid = fork();
				if (pid == 0){
					dup2(tuberia[1], STDOUT_FILENO);
					close(tuberia[1]);
					execvp(arg1[0], arg1);
					printf("¡¡ERROR!!");
				}
				else{
					dup2(tuberia[0], STDOUT_FILENO);
					close(tuberia[0]);
					execvp(arg2[0], arg2);
					printf("¡¡ERROR!!");
				}
			}
			else if(strcmp(arg1[i], ">") == 0){
				fid = open(arg1[i+1], O_WRONLY, 004);
				dup2(fid, STDOUT_FILENO);
				close(fid);
				free(arg1[i]);
				free(arg1[i+1]);
				arg1[i] = NULL;
				i++;
			}
			else if(strcmp(arg1[i], "<") == 0){
				fid = open(arg1[i+1], O_RDONLY, 040);
				dup2(fid, STDIN_FILENO);
				close(fid);
				free(arg1[i]);
				free(arg1[i+1]);
				arg1[i] = NULL;
				i++;
			}
			else if(strcmp(arg1[i], ">>") == 0){
				fid = open(arg1[i+1], O_WRONLY|O_APPEND, 060);
				dup2(fid, STDOUT_FILENO);
				close(fid);
				free(arg1[i]);
				free(arg1[i+1]);
				arg1[i] = NULL;
				i++;
				}
			i++;
			}
		execvp(arg1[0],arg);
		printf("¡¡ERROR!!\n"); 
		exit(888);
	    }
	    else wait(NULL);

    }
}
