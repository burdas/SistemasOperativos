//////////////////////////////
//        procshed.c        //
//////////////////////////////

/*
 * Nombre: procshed - Planificador
 * Autor: Marcos Burdaspar Celada
 * Sinopsis: procshed
 * Descripción: 
 				- El programa procsched crea las colas de mensajes y el planificador.
				- El programa procsched lanza los procesos que le han sido configurados y
					va encolando las peticiones de ejecución de los procesos en las colas
					de mensajes conforme recibe las solicitudes. El planificador va
					ejecutando los procesos encolados en las colas de mensajes atendiendo
					a la prioridad de cada nivel (la máxima prioridad corresponde al nivel
					más bajo).
				- El fichero de configuración configfile es opcional, de no estar presente
					se leerá la información de configuración de la entrada estándar,
					siguiendo el mismo formato que el fichero.
				- Formato del fichero de configuración: Consta de una línea por cada
					programa a ejecutar. Cada línea es de la siguiente forma:

					nivel nombreprograma argumento1 argumento 2 argumento 3 ...

				- Donde:
					nivel: indica la cola en la que se ha de encolar cada proceso.
					nombreprograma argumento1 argumento 2 argumento 3 ...: es el programa a
					ejecutar junto con sus argumentos, que pueden ser un número
					indeterminado y diferente para cada programa.
				- El programa procsched no finalizará hasta que el usuario pulse CTRL+C,
					momento en el que se terminará adecuadamente liberando todos los
					recursos empleados y se ofrecerá por pantalla el número de procesos
					que han concluido con éxito y el número de cambios de contexto
					realizados.
 * Valor devuelto:
 				El planificador procsched indicará a su finalización el número de
				procesos que han concluido con éxito y el número de cambios de contexto
				producidos.
 */
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

#define QUANTO 2					//Valor en segundos del tiempo de espera de la cola Round robin
#define TAM_ENTRADA 50				//Cantidad de caracteres como maximo que puede leer hasta el salto de linea

void error(char *cadena);			//Función que devuelve error y sale del programa
void acabar(int y);					//Función que envia al proceso padre la señal SIGUSR1
void manejador(int y);				//Función que pone a uno la variable estado
void finalizar(int y);				//Función que pone a uno la variable finalizado

int estado, finalizado;

typedef struct msgbuf{
	long id;						//Nivel de prioridad
	int pid;						//Pid del proeso
}msgbuf; 

void main(int argc, char const *argv[]){
	int primerPid, msqid, status, cambioContexto = 0, progfin = 0;
	struct msgbuf mensaje;
	key_t clave;

	if (argc > 2)
	{
		error("Número de argumentos mayor que uno...");
	}

	finalizado = 0;

	clave = ftok("/etc", 22);
	if (clave == (key_t) -1)
	{
		error("No se pudo obtener la clave");
	}

	msqid = msgget (clave, 0600 | IPC_CREAT);
	if (msqid == -1) {
		error("Problema en la creación de la cola de mensajes");
	}

	struct sigaction accion;
	
	accion.sa_flags = SA_NOCLDSTOP | SA_RESTART;							//No detecta si el hijo se para, por cada hijo se resetea
	accion.sa_handler = &acabar;											//Función manejador acabar


	primerPid = fork();														//Padre planificador, hijo lector e invocador de procesos

	if (primerPid == -1)
	{
		error("No se pudo crear el primer hijo...");
	}

	if (primerPid == 0)
	{
		sigaction(SIGCHLD, &accion, NULL);									//Señal que se activa cuando se termine un hijo
		// Primer hijo
		char *cadena, **arg;
		int tipo;
		int pid;

		if (argc == 2)														//Si se introduce un fichero se cambia el
		{																	//el descriptor de fichero entrada de datos
			int fp;
			fp = open(argv[1], O_RDONLY);
			if (fp == -1){
				error("No se pudo abrir el ficehro...");
			}
			dup2(fp, STDIN_FILENO);
			close(fp);
		}

		cadena = (char *)malloc(TAM_ENTRADA);								//Se reserva memoria segun el tamaño de entrada

		printf("\nCaptura de Procesos\n");
		printf("---------------------------------------------------------\n");
		while(scanf("%d %[^\n]", &tipo, cadena) != EOF){						//Luego el comando y sus argumentos
			getchar();

			arg = fragmenta(cadena);

			pid = fork();

			if (pid == -1)
			{
				error("Problema en la creación del proceso hijo");
			}

			if (pid == 0)
			{
				//hijo

				kill(getpid(), SIGSTOP);

				execvp(arg[0], arg);
				printf("No se reconoce el comando << %s >>\n", arg[1]);
				exit(EXIT_SUCCESS);
			}
			sleep(1);
			mensaje.id = (long)(tipo);
			mensaje.pid = pid;
			msgsnd(msqid, &mensaje, sizeof(int), IPC_NOWAIT);
			printf("Introducido en la cola de mensajes proceso: %d de tipo: %ld\n", mensaje.pid, mensaje.id);
		}

		while(1){						//Se mantiene activo para recibir las señales
			sleep(100);
		}

	} else {
		// Padre del primer fork

		signal(SIGINT, &finalizar);							//Señal que se activa con Crt-c
		signal(SIGUSR1, &manejador);						//Señal que se activa con la funcion acabar
		int pidAnterior = -1;
		
		while(finalizado == 0){
			estado = 0;
			if (msgrcv(msqid, &mensaje, sizeof(int), 1, IPC_NOWAIT) != EOF)
			{
				if(pidAnterior != mensaje.pid){
					cambioContexto++;
				}
				printf("Pid: <[%d]> en cola 1\n", mensaje.pid);
				kill(mensaje.pid, SIGCONT);
				sleep(QUANTO);
				if (estado == 0){
					kill(mensaje.pid, SIGSTOP);
					msgsnd(msqid, &mensaje, sizeof(int), IPC_NOWAIT);
				} else {
					progfin++;
				}
				pidAnterior = mensaje.pid;
			} else {
				if (msgrcv(msqid, &mensaje, sizeof(int), 2, IPC_NOWAIT) != EOF)
				{
					if(pidAnterior != mensaje.pid){
						cambioContexto++;
					}
					printf("Pid: <[%d]> en cola 2\n", mensaje.pid);
					kill(mensaje.pid, SIGCONT);

					waitpid(mensaje.pid, 0, 0);

					if (estado != 0)
					{
						progfin++;
					}

					pidAnterior = mensaje.pid;
				}
			}
		}
		if (finalizado == 1)
		{
			printf("\nInterrupción del planificador \n");
			printf("---------------------------------------------------------\n");
			printf("Ha habido %d cambios de contexto y han finalizado correctamente %d procesos.\n", cambioContexto, progfin);
			printf("Liberando los recursos...\n");
			msgctl(msqid, IPC_RMID, 0);
			exit(EXIT_SUCCESS);
		}
	}

}

void error(char *cadena){
	printf("[ERROR] %s\n", cadena);
	exit(EXIT_FAILURE);
}

void acabar(int y){
	kill(getppid(), SIGUSR1);
}

void manejador(int y){
	estado = 1;
}

void finalizar(int y){
	finalizado = 1;
}