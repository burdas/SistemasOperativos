/* ----------------------------------------------------------------------------------
 * Programa: main.c
 * Función: Se encarga de planificar un conjunto de procesos
 *          mediante la técnica round robin con un quanto introducido por el usuario
 * Entrada: quanto y un fichero con procesos escritos en cada línea
 *          también se puede omitir el fichero e introducir directamente los procesos
 * Salida:  Cantidad de cambios de contexto y ciclos que se han ejecutado
 * Autor:   Cristian Berner
 * Fecha:   25/11/2016
 * Función de compilación: make
 * Librerías propias: fragmenta.h
 * ----------------------------------------------------------------------------------
*/
//-------------------------//
//      Importaciones      //
//-------------------------//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/signal.h>
#include <wait.h>
#include <string.h>
#include <fcntl.h>
#include "fragmenta.h"

//----------------------------------//
//Declaración de variables globales,//
//estructuras y funciones.          //
//----------------------------------//
struct msgStruct{
	long id;
	int pid;
}msgStruct;

int estado, finalizado;
void acabar(int y);
void finalizar(int y);

//------------------------------//
//       Función principal      //
//------------------------------//
void main(int argc, char** argv){
	
	if (argc < 3){
		printf("Argumentos incorrectos (main quanto ficheroprocesos)");
		exit(EXIT_FAILURE);
	}	
	int idFichero, cambiosDeContexto, ciclos, pidAnterior, pid, quanto, key, msqid;
	struct msgStruct mensaje;
	//Obtengo el cuanto del primer argumento y la id de la cola junto con una clave
	quanto = atoi(argv[1]);
	key = ftok("/etc", 5);
	msqid = msgget(key, IPC_CREAT|0600);
	//En el buffer se irá leyendo línea a línea la entrada
	char *buffer = (char*)malloc(200 * sizeof(char));
	
	//Inicio las variables, usaré como id de mensaje 4
	cambiosDeContexto = 0;
	ciclos = 0;
	pidAnterior = -1;
	finalizado = 0;
	mensaje.id = 4;
	
	//De haberse introducido un fichero como argumento, lo abro en lectura
	//duplico el descriptor
	//de fichero en la entrada del programa, finalmente cerrandolo
	if(argc == 3){
		idFichero = open(argv[2], O_RDONLY);
		dup2(idFichero, STDIN_FILENO);
		close(idFichero);
	}
	msgget(4, IPC_CREAT|IPC_PRIVATE|0600);
	while(scanf("%[^\n]", buffer) != EOF){
		pid = fork();
		if (pid == -1){
			printf("[ERROR]Creacion del hijo\n");
			exit(EXIT_FAILURE);
		}
		
		if (pid == 0){
			//Hijo
			//Obtengo los argumentos fragmentando el buffer;
			char **args;
			args = fragmenta(buffer);
			
			//Paro el proceso enviandole una señal de SIGSTOP
			//Después ejecuto el proceso correspondiente
			kill(getpid(), SIGSTOP);
			execvp(args[0], args);
			
			//El error será el pid del proceso para poder verificarlo después
			exit(getpid()); 
		}else{
			//Padre, se encargará de guardar su pid y enviarlo a la cola de mensajes
			mensaje.pid = pid;
			msgsnd(msqid, &mensaje, sizeof(int), IPC_NOWAIT);
		}
		getchar();
	}

	
	//Creo una variable del tipo estructura sigaction, para poder manejar las señales
	struct sigaction accion;
	
	accion.sa_flags = SA_NOCLDSTOP;
	accion.sa_handler = &acabar;
	
	//Asigno a la señal SIGCHLD la función accion y 
	//a la señal SIGINT la función finalizar
	sigaction(SIGCHLD, &accion, NULL);
	signal(SIGINT, &finalizar);
	
	//Mientras haya elementos en la cola, aumento el ciclo y verifico si
	//hay cambio de contexto, de haberlo aumento la variable
	//finalmente envio una señal de continuación al proceso y espero el quanto
	//Si el proceso ha terminado, es decir, ha enviado la señal SIGCHLD, la 
	//variable estado habrá cambiado y no volveré a meter al proceso en la cola
	//de mensajes.
	//En caso de que el proceso no haya terminado, le enviaré una señal SIGSTOP
	//al padre y volveré a introducirlo en la cola de mensajes
	//Finalmente actualizaré la variable pidAnterior al pid ejecutado recientemente
	while ((msgrcv(msqid, &mensaje, sizeof(int), mensaje.id, IPC_NOWAIT) != EOF) && (!finalizado)){
		ciclos++;
		estado = 0;
		if(pidAnterior != mensaje.pid){
			cambiosDeContexto++;
		}
		kill(mensaje.pid, SIGCONT);
		sleep(quanto);
		if (!estado){
			kill(mensaje.pid, SIGSTOP);
			msgsnd(msqid, &mensaje, sizeof(int), IPC_NOWAIT);
		}
		pidAnterior = mensaje.pid;
	}

	//Una vez finalizado enviaré una señal a cada uno de los procesos de la cola
	//de mensajes de tipo SIGTERM para que se acaben completamente
	if (finalizado){
		while(msgrcv(msqid, &mensaje, sizeof(int), mensaje.id, IPC_NOWAIT) != EOF){
			kill(mensaje.pid, SIGTERM);
		}
	}
	printf("Han habido %d ciclos y %d cambios de contexto.\n", ciclos, cambiosDeContexto);
	
	//Por último elimino la cola de mensajes
	msgctl(msqid, IPC_RMID, 0);
}

//La función acabar se encarga de modificar el estado y se ejecutará al 
//recibir un proceso la señal SIGCHLD
void acabar(int y){
	estado = 1;
}

//La función finalizar se encargará de avisar cuando ya no hay más procesos para ejecutarse
void finalizar(int y){
	finalizado = 1;
}
