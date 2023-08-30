/////////////////////////////
//          msh.c          //
/////////////////////////////

/*
 * Nombre: minishell - Mini Shell
 * Autor: Marcos Burdaspar Celada
 * Sinopsis: msh
 * Descripción: Las funcionalidades de las shell son:
                    [x] Ejecución de comandos con un número indeterminados de argumentos.
                    [x] Redirección de la salida estándar de fichero mediante >
                    [x] Redirección con anexión de la salida estándar de fichero mediante >>
                    [x] Redirección de la entrada estándar de fichero mediante <
                    [x] Redirección simultanea de entrada y salida estandar en cualquier orden.
                    [x] Redirección de la salida de un proceso a la entrada de otro mediante |
                    [x] Prompt personalizado "minishell\>"
                    [x] Conclusión de la shell con Ctrl+C o con el comando "exit"

 */

//--------------------------------------------------------//
// Definición de librerías a utilizar dentro del programa //
//--------------------------------------------------------//
/*
 * stdio      -> entrada/salida estandar
 * stdlib     -> funcionas básicas estándar
 * string     -> manejo de strings con strcmp, strcpy, strcat...
 * unistd     -> funciones para el manejo de ficheros
 * fcntl      -> file control options (Opciones de control de ficheros)
 * fragmenta  -> transformar una cadena en una estructura de doble puntero
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "fragmenta.h"

#define TAM_ESCRITURA 50
//Como no sabemos con anterioridad el numero de caracteres que va a escribir el usuario,
//establecemos tamaño 50;

int main(void) { 
    int pid;
    char *cadena;
    cadena = (char *)malloc(TAM_ESCRITURA);// Como el tamaño de char es 1 byte obviamos el sizeof(char)
    system("clear");

    while(1){
        printf("minishell\\> ");            // Prompt personalizado
        scanf("%[^\n]",cadena);             // Leer hasta salto de linea
        getchar();                          // Eliminar el caracter de salto de linea del buffer de entrada

        if (strcmp(cadena, "exit") == 0){
            exit(EXIT_SUCCESS);             
        } else {
           
            pid = fork();
               
            if(pid == -1){
                printf("[Error]Creacion hijo");
                exit(EXIT_FAILURE);
            }

            if(pid == 0){ 
                //Hijo
                char **arg;                         
                int i = 0;
                arg = fragmenta(cadena);

                
                //Busqueda | dentro de la cadena
                
                while (arg[i] != NULL && strcmp(arg[i], "|")!= 0){
                    i++;
                }
                
                //Caso de que no haya pipe

                if (arg[i] == NULL){

                    //Busco redirecciones a archivos o desde archivos y modifico el descriptor
                    //de fichero de entrada o salida del proceso dependiendo de ello

                    i = 0;
                    while (arg[i] != NULL){
                        if (strcmp(arg[i], ">") == 0){
                            int fid = open(arg[i + 1], O_WRONLY|O_CREAT, 0600);
                            dup2(fid, STDOUT_FILENO);
                            close(fid);

                            free(arg[i]);           
                            free(arg[i + 1]);

                            //Liberamos la memoria del apuntador al simbolo ">" y de la palabra
                            //siguiente, que es el nombre del fichero

                            arg[i] = NULL;
                            arg[i + 1] = NULL;

                            //Hacemos que apunten a NULL para que la función posterior execvp()
                            //no este buscando mas argumentos

                            i++;

                            //Aumento uno porque hemos leido el simbolo y el nombre del fichero

                            //Este proceso se repite en las demás redirecciones

                        }else if(strcmp(arg[i], ">>") == 0){
                            int fid = open(arg[i + 1], O_WRONLY|O_APPEND|O_CREAT, 0600);
                            dup2(fid, STDOUT_FILENO);
                            close(fid);

                            free(arg[i]);
                            free(arg[i + 1]);
                            arg[i] = NULL;
                            arg[i + 1] = NULL;
                            i++;
                        }else if(strcmp(arg[i], "<") == 0){
                            int fid = open(arg[i + 1], O_RDONLY, 0600);
                            if (fid != -1)
                                dup2(fid, STDIN_FILENO);
                            close(fid);

                            free(arg[i]);
                            free(arg[i + 1]);
                            arg[i] = NULL;
                            arg[i + 1] = NULL;
                            i++;
                        }

                        i++;
                    }

                    execvp(arg[0], arg);

                    //Si no se reconoce el comando se notifica por pantalla
                    printf("Comando << %s >> no reconocido\n", arg[0]);
                    borrarg(arg);
                }else{

                    //Caso de encontrar pipe |, procedemos a crear otro hijo y una pipe

                    int pipes[2];
                    int pid2;
                    
                    if (pipe(pipes) == -1){
                        printf("[Error]Creacion de tuberias");
                        exit(EXIT_FAILURE);
                    }
                    
                    pid2 = fork();

                    if (pid2 == -1){
                        printf("[Error]Creacion del segundo hijo");
                        exit(EXIT_FAILURE);
                    }
                    
                    
                    if (pid2 == 0){
                        //Segundo hijo, primer miembro del comando
                        
                        arg[i] = NULL;  //Símbolo | apuntado a NULL, para que execvp deje de buscar

                        close(pipes[0]); //Cierro la tuberia de lectura
                        dup2(pipes[1], STDOUT_FILENO);

                        //Se redirecciona la salida del proceso a la tuberia de escritura

                        close(pipes[1]);

                        execvp(arg[0], arg);
                    }else{
                        //Padre del segundo hijo, segundo miembro del comando
                        
                        char **arg2;
                        arg2 = &arg[i + 1];

                        //Hacemos una copia de la parte posterior a la pipe

                        close(pipes[1]); //Cierro la tuberia de escritura
                        dup2(pipes[0], STDIN_FILENO);

                        //Se redirecciona la entrada del proceso a la tuberia de lectura

                        close(pipes[0]);
                        execvp(arg2[0], arg2);
                        wait(NULL);
                    }
                }

                exit(EXIT_SUCCESS); //Terminamos el primer proceso hijo
                
            }else{
                //Padre
                wait(NULL);
                
            }
        }
    }
    return (EXIT_SUCCESS);
}


