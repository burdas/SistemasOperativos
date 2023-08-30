/*Librerias para votante*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/*Programa votante*/
int main(int argc, char **argv)
{
	/*Variables*/
	int periodo, numVotos, i, usuarioElegido;
	i=0;
	/*Recogemos con atoi los argumentos periodo 
	 * y numVotos para conventirlos a entero*/
	periodo=atoi(argv[1]);
	numVotos=atoi(argv[2]);
	/*funcion srand(): nos permitira crear numeros aleatorios sin
	 * que se repita la secuencia asignandole getpid 	que nos 
	 dara un entero diferente cada vez que se ejecute el programa*/
	srand(getpid());
	/*Bucle para crear tantos votos como numvotos*/
	while(i<=numVotos){
		sleep(periodo);
		/*generar voto con la funcion random*/
		usuarioElegido=(rand()%4)+1;
        write(STDOUT_FILENO,&usuarioElegido,sizeof(int));
		i++;
	}
}